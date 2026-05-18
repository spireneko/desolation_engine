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

	// Sphere for point lights
	std::vector<Vector3> sphereVerts;
	std::vector<uint32_t> sphereInds;
	const int slices = 32, stacks = 32;

	for (int stack = 0; stack <= stacks; ++stack) {
		float phi = DirectX::XM_PI * stack / stacks;
		float y = 0.5f * cosf(phi);
		float r = 0.5f * sinf(phi);
		for (int slice = 0; slice <= slices; ++slice) {
			float theta = 2.0f * DirectX::XM_PI * slice / slices;
			sphereVerts.emplace_back(r * cosf(theta), y, r * sinf(theta));
		}
	}
	for (int stack = 0; stack < stacks; ++stack) {
		int row = stack * (slices + 1);
		int nextRow = (stack + 1) * (slices + 1);
		for (int slice = 0; slice < slices; ++slice) {
			sphereInds.push_back(row + slice);
			sphereInds.push_back(nextRow + slice);
			sphereInds.push_back(row + slice + 1);
			sphereInds.push_back(nextRow + slice);
			sphereInds.push_back(nextRow + slice + 1);
			sphereInds.push_back(row + slice + 1);
		}
	}
	sphereIndexCount = static_cast<UINT>(sphereInds.size());

	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vector3) * sphereVerts.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA vInit = {sphereVerts.data()};
	device->CreateBuffer(&vbd, &vInit, &sphereVertexBuffer);

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * sphereInds.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA iInit = {sphereInds.data()};
	device->CreateBuffer(&ibd, &iInit, &sphereIndexBuffer);

	// Cone for spot lights (apex at origin, base at y=1, radius=1, pointing up +Y)
	std::vector<Vector3> coneVerts;
	std::vector<uint32_t> coneInds;
	const int coneSlices = 32;

	coneVerts.emplace_back(0.0f, 0.0f, 0.0f);  // apex
	coneVerts.emplace_back(0.0f, 1.0f, 0.0f);  // base center

	for (int i = 0; i <= coneSlices; ++i) {
		float theta = 2.0f * DirectX::XM_PI * i / coneSlices;
		coneVerts.emplace_back(cosf(theta), 1.0f, sinf(theta));
	}

	for (int i = 0; i < coneSlices; ++i) {
		int base = 2 + i;
		int next = 2 + ((i + 1) % coneSlices);
		// Side (outward winding — will be rendered with front-face culling)
		coneInds.push_back(0);
		coneInds.push_back(next);
		coneInds.push_back(base);
		// Base cap
		coneInds.push_back(1);
		coneInds.push_back(base);
		coneInds.push_back(next);
	}

	coneIndexCount = static_cast<UINT>(coneInds.size());

	D3D11_BUFFER_DESC coneVbd = {};
	coneVbd.Usage = D3D11_USAGE_DEFAULT;
	coneVbd.ByteWidth = static_cast<UINT>(sizeof(Vector3) * coneVerts.size());
	coneVbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA coneVInit = {coneVerts.data()};
	device->CreateBuffer(&coneVbd, &coneVInit, &coneVertexBuffer);

	D3D11_BUFFER_DESC coneIbd = {};
	coneIbd.Usage = D3D11_USAGE_DEFAULT;
	coneIbd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * coneInds.size());
	coneIbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA coneIInit = {coneInds.data()};
	device->CreateBuffer(&coneIbd, &coneIInit, &coneIndexBuffer);

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
	sphereVertexBuffer.Reset();
	sphereIndexBuffer.Reset();
	coneVertexBuffer.Reset();
	coneIndexBuffer.Reset();
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
	auto* shaders = gameContext->GetShaderManager();

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

	gBuffer->BindForGeometryPass(ctx);
	gBuffer->Clear(ctx, Colors::DarkGray);

	PerFrameConstants constants;
	constants.view = view.Transpose();
	constants.projection = proj.Transpose();
	constants.cameraPosition = cameraPos;
	constants.dirLight = lightManager->GetDirectionalLight();
	lightManager->PrepareLights(constants, cameraPos);
	constants.shadowData = lightManager->GetShadowConstants();

	shaders->Apply(ShaderManager::PassType::GBuffer);
	shaders->UpdateConstants(ShaderManager::PassType::GBuffer, &constants, sizeof(constants));

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

	graphics->BeginFrame(Colors::Black);

	ID3D11ShaderResourceView* gbuffers[4] = {
		gBuffer->GetSRV(GBuffer::Attachment::AlbedoMetallic),
		gBuffer->GetSRV(GBuffer::Attachment::NormalRoughness),
		gBuffer->GetSRV(GBuffer::Attachment::EmissiveAO),
		gBuffer->GetSRV(GBuffer::Attachment::Depth)
	};
	ctx->PSSetShaderResources(0, 4, gbuffers);

	lightManager->BindShadowMap(ctx, 4);

	LightingConstants litConstants;
	litConstants.cameraPosition = camera->position;
	litConstants.cascadeCount = lightManager->GetShadowConstants().cascadeCount;
	litConstants.dirLight = lightManager->GetDirectionalLight();

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();
	litConstants.invProj = proj.Invert().Transpose();
	litConstants.invViewProj = (view * proj).Invert().Transpose();
	litConstants.shadowData = lightManager->GetShadowConstants();
	litConstants.screenSize =
		Vector2(static_cast<float>(gBuffer->GetWidth()), static_cast<float>(gBuffer->GetHeight()));
	// litConstants.debugCascades = lightManager->GetDebugShadowCascades() ? 1 : 0;
	litConstants.debugCascades = 0;

	ctx->OMSetDepthStencilState(lightingDSS.Get(), 0);
	ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	shaders->Apply(ShaderManager::PassType::DeferredDirectional);
	shaders->UpdateConstants(ShaderManager::PassType::DeferredDirectional, &litConstants, sizeof(litConstants));
	fsQuad->Draw(ctx);

	// Point lights (additive)
	ctx->OMSetBlendState(additiveBlendState.Get(), nullptr, 0xFFFFFFFF);
	ctx->OMSetDepthStencilState(lightingDSS.Get(), 0);

	const auto& pointLights = lightManager->GetCachedLights();
	ExecutePointLights(pointLights, litConstants.invViewProj, camera->position, camera);

	// Spot lights (additive)
	const auto& spotLights = lightManager->GetCachedSpotLights();
	ExecuteSpotLights(spotLights, litConstants.invViewProj, camera->position, camera);

	ID3D11ShaderResourceView* nullsrvs[6] = {nullptr};
	ctx->PSSetShaderResources(0, 6, nullsrvs);

	ctx->RSSetState(nullptr);
	ctx->OMSetDepthStencilState(nullptr, 0);
	ctx->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void RenderingSystem::ExecutePointLights(
	const std::vector<LightData::PointLight>& points, const Matrix& invViewProj, const Vector3& cameraPos,
	CameraComponent* camera
)
{
	auto ctx = gameContext->GetGraphicsContext();
	auto shaders = gameContext->GetShaderManager();

	shaders->Apply(ShaderManager::PassType::DeferredPoint);

	ctx->OMSetBlendState(additiveBlendState.Get(), nullptr, 0xFFFFFFFF);
	ctx->OMSetDepthStencilState(volumeDSS.Get(), 0);

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();
	Vector2 screenSize(static_cast<float>(gBuffer->GetWidth()), static_cast<float>(gBuffer->GetHeight()));

	for (const auto& pl : points) {
		bool cameraInside = Vector3::Distance(cameraPos, pl.position) < pl.range;

		if (cameraInside) {
			ctx->RSSetState(nullptr);
		} else {
			ctx->RSSetState(volumeRS.Get());
		}

		Matrix world = Matrix::CreateScale(pl.range) * Matrix::CreateTranslation(pl.position);
		Matrix wvp = world * view * proj;

		PointLightConstants pc;
		pc.pointLight = pl;
		pc.invViewProj = invViewProj;
		pc.cameraPosition = cameraPos;
		pc.worldViewProj = wvp.Transpose();
		pc.screenSize = screenSize;

		shaders->UpdateConstants(ShaderManager::PassType::DeferredPoint, &pc, sizeof(pc));
		DrawLightVolume(ctx);
	}

	ctx->RSSetState(nullptr);
	ctx->OMSetDepthStencilState(nullptr, 0);
}

void RenderingSystem::ExecuteSpotLights(
	const std::vector<LightData::SpotLight>& spots, const Matrix& invViewProj, const Vector3& cameraPos,
	CameraComponent* camera
)
{
	auto ctx = gameContext->GetGraphicsContext();
	auto shaders = gameContext->GetShaderManager();

	shaders->Apply(ShaderManager::PassType::DeferredSpot);

	ctx->OMSetBlendState(additiveBlendState.Get(), nullptr, 0xFFFFFFFF);
	ctx->OMSetDepthStencilState(volumeDSS.Get(), 0);

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();
	Vector2 screenSize(static_cast<float>(gBuffer->GetWidth()), static_cast<float>(gBuffer->GetHeight()));

	for (const auto& sl : spots) {
		// Conservative "camera inside cone" test
		Vector3 toCamera = cameraPos - sl.position;
		float dist = toCamera.Length();
		bool cameraInside = false;
		if (dist < sl.range && dist > 0.001f) {
			float cosAngle = (toCamera / dist).Dot(sl.direction);
			if (cosAngle > cosf(sl.outerAngle)) {
				cameraInside = true;
			}
		}

		if (cameraInside) {
			ctx->RSSetState(nullptr);
		} else {
			ctx->RSSetState(volumeRS.Get());
		}

		Vector3 forward = sl.direction;
		forward.Normalize();

		Vector3 up = Vector3::Up;
		if (std::abs(up.Dot(forward)) > 0.999f) {
			up = Vector3::Forward;
		}
		Vector3 right = up.Cross(forward);
		right.Normalize();
		up = forward.Cross(right);

		// clang-format off
		Matrix rot(
			right.x,	right.y,	right.z, 	0.0f,
			forward.x,	forward.y,	forward.z, 	0.0f,
			up.x,		up.y, 		up.z,		0.0f,
			0.0f,		0.0f,		0.0f,		1.0f
		);
		// clang-format on

		float tanOuter = tanf(sl.outerAngle);
		Matrix scale = Matrix::CreateScale(sl.range * tanOuter, sl.range, sl.range * tanOuter);
		Matrix world = scale * rot * Matrix::CreateTranslation(sl.position);
		Matrix wvp = world * view * proj;

		SpotLightConstants sc = {};
		sc.position = sl.position;
		sc.intensity = sl.intensity;
		sc.color = sl.color;
		sc.range = sl.range;
		sc.direction = sl.direction;
		sc.innerAngle = sl.innerAngle;
		sc.outerAngle = sl.outerAngle;
		sc.constant = sl.constant;
		sc.linearAttenuation = sl.linear;
		sc.quadratic = sl.quadratic;
		sc.invViewProj = invViewProj;
		sc.cameraPosition = cameraPos;
		sc.worldViewProj = wvp.Transpose();
		sc.screenSize = screenSize;

		shaders->UpdateConstants(ShaderManager::PassType::DeferredSpot, &sc, sizeof(sc));
		DrawConeVolume(ctx);
	}

	ctx->RSSetState(nullptr);
	ctx->OMSetDepthStencilState(nullptr, 0);
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

			shaders->UpdatePerObjectConstants(&obj, sizeof(obj));
			current->Draw();
		}

		for (auto& child : current->GetChildren()) {
			queue.push_back(child);
		}
	}
}

void RenderingSystem::DrawLightVolume(ID3D11DeviceContext* ctx)
{
	UINT stride = sizeof(Vector3);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, sphereVertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(sphereIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->DrawIndexed(sphereIndexCount, 0, 0);
}

void RenderingSystem::DrawConeVolume(ID3D11DeviceContext* ctx)
{
	UINT stride = sizeof(Vector3);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, coneVertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(coneIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->DrawIndexed(coneIndexCount, 0, 0);
}
