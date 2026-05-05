#include "RenderingSystem.hpp"

#include <deque>

#include "Mesh.hpp"

RenderingSystem::RenderingSystem(GameContext* ctx) : gameContext(ctx) {}

bool RenderingSystem::Initialize(int width, int height)
{
	auto device = gameContext->GetGraphicsDevice();

	gBuffer = std::make_unique<GBuffer>();
	if (!gBuffer->Initialize(device, width, height)) {
		return false;
	}

	fsQuad = std::make_unique<FullscreenQuad>();
	if (!fsQuad->Initialize(device)) {
		return false;
	}

	// Additive blend for light accumulation
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&blendDesc, &additiveBlendState);

	// Depth stencil states
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = FALSE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.StencilEnable = FALSE;
	device->CreateDepthStencilState(&dsDesc, &lightingDSS);

	D3D11_DEPTH_STENCIL_DESC volumeDesc = {};
	volumeDesc.DepthEnable = TRUE;
	volumeDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	volumeDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	volumeDesc.StencilEnable = FALSE;
	device->CreateDepthStencilState(&volumeDesc, &volumeDSS);

	// Cull front for light volumes (render inside)
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_FRONT;
	rsDesc.DepthClipEnable = TRUE;
	device->CreateRasterizerState(&rsDesc, &volumeRS);

	// Create simple sphere for point light volumes
	// (Simplified: use 16x16 sphere, or load from mesh)
	// For now, we'll use a simple approach - create in code or use existing Mesh::CreateSphere

	initialized = true;
	return true;
}

void RenderingSystem::Shutdown()
{
	if (gBuffer) {
		gBuffer->Shutdown();
	}
	fsQuad.reset();
	additiveBlendState.Reset();
	lightingDSS.Reset();
	volumeDSS.Reset();
	volumeRS.Reset();
	initialized = false;
}

void RenderingSystem::Resize(int width, int height)
{
	if (gBuffer) {
		gBuffer->Resize(width, height);
	}
}

void RenderingSystem::RenderFrame(
	const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, CameraComponent* camera, LightManager* lightManager
)
{
	if (!initialized || !camera) {
		return;
	}

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();
	Vector3 cameraPos = camera->position;

	// 1. Shadow pass
	auto dirLight = lightManager->GetDirectionalLight();
	ExecuteShadowPass(
		sceneRoots,
		dirLight.direction,
		view,
		camera->GetFOVRadians(),
		camera->GetAspectRatio(),
		camera->GetNearPlane(),
		camera->GetFarPlane()
	);

	// 2. GBuffer pass
	ExecuteGBufferPass(sceneRoots, view, proj, cameraPos, lightManager);

	// 3. Deferred lighting
	ExecuteDeferredLighting(camera, lightManager);

	// 4. Forward pass for transparent objects (if any)
	// ExecuteForwardPass(transparentObjects, view, proj);
}

void RenderingSystem::ExecuteShadowPass(
	const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, const Vector3& lightDir, const Matrix& cameraView,
	float fovRadians, float aspect, float nearPlane, float farPlane
)
{
	auto* lightManager = gameContext->GetLightManager();
	auto* shaders = gameContext->GetShaderManager();  // Need to expose this

	// This delegates to LightManager's existing shadow rendering
	lightManager->RenderShadowCascades(
		gameContext->GetGraphicsContext(),
		gameContext,
		shaders,
		sceneRoots,
		lightDir,
		cameraView,
		fovRadians,
		aspect,
		nearPlane,
		farPlane
	);
}

void RenderingSystem::ExecuteGBufferPass(
	const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, const Matrix& view, const Matrix& proj,
	const Vector3& cameraPos, LightManager* lightManager
)
{
	auto ctx = gameContext->GetGraphicsContext();
	auto shaders = gameContext->GetShaderManager();

	// Bind GBuffer
	gBuffer->BindForGeometryPass(ctx);
	gBuffer->Clear(ctx, Colors::DarkGray);

	// Setup constants
	PerFrameConstants constants;
	constants.view = view.Transpose();
	constants.projection = proj.Transpose();
	constants.cameraPosition = cameraPos;
	constants.dirLight = lightManager->GetDirectionalLight();
	lightManager->PrepareLights(constants, cameraPos);
	constants.shadowData = lightManager->GetShadowConstants();

	shaders->Apply(ShaderManager::PassType::GBuffer);
	shaders->UpdateConstants(ShaderManager::PassType::GBuffer, &constants, sizeof(constants));

	// Draw all opaque geometry
	for (auto& root : sceneRoots) {
		if (!root->GetParent()) {
			DrawSceneGeometry({root}, ShaderManager::PassType::GBuffer);
		}
	}
}

void RenderingSystem::ExecuteDeferredLighting(CameraComponent* camera, LightManager* lightManager)
{
	auto ctx = gameContext->GetGraphicsContext();
	auto shaders = gameContext->GetShaderManager();
	auto graphics = gameContext->GetGraphics();

	// Restore backbuffer
	graphics->BeginFrame(Colors::Black);  // We'll accumulate light, so start black

	// Setup GBuffer SRVs
	ID3D11ShaderResourceView* gbuffers[4] = {
		gBuffer->GetSRV(GBuffer::Attachment::AlbedoMetallic),
		gBuffer->GetSRV(GBuffer::Attachment::NormalRoughness),
		gBuffer->GetSRV(GBuffer::Attachment::EmissiveAO),
		gBuffer->GetSRV(GBuffer::Attachment::Depth)
	};
	ctx->PSSetShaderResources(0, 4, gbuffers);

	// Bind shadow map
	lightManager->BindShadowMap(ctx, 4);

	// Directional light (full screen)
	LightingConstants litConstants;
	litConstants.cameraPosition = camera->position;
	litConstants.cascadeCount = lightManager->GetShadowConstants().cascadeCount;
	litConstants.dirLight = lightManager->GetDirectionalLight();

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();
	litConstants.invViewProj = (view * proj).Invert().Transpose();
	litConstants.shadowData = lightManager->GetShadowConstants();
	litConstants.screenSize =
		Vector2(static_cast<float>(gBuffer->GetWidth()), static_cast<float>(gBuffer->GetHeight()));

	ctx->OMSetDepthStencilState(lightingDSS.Get(), 0);
	ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);	 // No blending for first light

	shaders->Apply(ShaderManager::PassType::DeferredDirectional);
	shaders->UpdateConstants(ShaderManager::PassType::DeferredDirectional, &litConstants, sizeof(litConstants));
	fsQuad->Draw(ctx);

	// Point lights (additive)
	ctx->OMSetBlendState(additiveBlendState.Get(), nullptr, 0xFFFFFFFF);
	ctx->OMSetDepthStencilState(volumeDSS.Get(), 0);
	ctx->RSSetState(volumeRS.Get());

	std::vector<LightData::PointLight> pointLights;
	// Extract from lightManager...
	ExecutePointLights(pointLights, litConstants.invViewProj, camera->position);

	// Spot lights (additive)
	// ExecuteSpotLights(...);

	// Cleanup SRVs to avoid D3D11 warnings
	ID3D11ShaderResourceView* nullsrvs[5] = {nullptr};
	ctx->PSSetShaderResources(0, 5, nullsrvs);

	// Restore states
	ctx->RSSetState(nullptr);
	ctx->OMSetDepthStencilState(nullptr, 0);
	ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void RenderingSystem::ExecutePointLights(
	const std::vector<LightData::PointLight>& points, const Matrix& invViewProj, const Vector3& cameraPos
)
{
	auto ctx = gameContext->GetGraphicsContext();
	auto shaders = gameContext->GetShaderManager();

	shaders->Apply(ShaderManager::PassType::DeferredPoint);

	for (const auto& pl : points) {
		PointLightConstants pc;
		pc.pointLight = pl;
		pc.invViewProj = invViewProj;
		pc.cameraPosition = cameraPos;

		shaders->UpdateConstants(ShaderManager::PassType::DeferredPoint, &pc, sizeof(pc));

		// Render light volume (sphere at light position with radius = range)
		// For now, use fullscreen quad as fallback, or implement sphere rendering
		fsQuad->Draw(ctx);	// Fallback: full screen with discard in shader
	}
}

void RenderingSystem::DrawSceneGeometry(
	const std::vector<std::shared_ptr<GameComponent>>& roots, ShaderManager::PassType pass
)
{
	auto shaders = gameContext->GetShaderManager();

	std::deque<std::shared_ptr<GameComponent>> queue;
	for (auto& r : roots) {
		queue.push_back(r);
	}

	while (!queue.empty()) {
		auto current = queue.front();
		queue.pop_front();

		if (current->isActive && current->GetMesh()) {
			Matrix world = current->GetWorldMatrix();

			PerObjectConstants obj;
			obj.world = world.Transpose();
			obj.material = current->GetMaterial();
			// View/Proj set outside for efficiency

			shaders->UpdatePerObjectConstants(&obj, sizeof(obj));
			current->Draw();
		}

		for (auto& child : current->GetChildren()) {
			queue.push_back(child);
		}
	}
}
