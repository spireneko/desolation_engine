#include "LightManager.hpp"

#include <deque>

#include "GameComponent.hpp"
#include "GameContext.hpp"
#include "PointLight.hpp"
#include "ShaderManager.hpp"

LightManager::LightManager()
{
	directionalLight.direction = Vector3(1.3, -1.0, 0.2);
	directionalLight.direction.Normalize();
	directionalLight.color = Vector3(1.0, 1.0, 1.0);
	directionalLight.intensity = 1.0;
}

void LightManager::Initialize(ID3D11Device* device)
{
	shadowMap = std::make_unique<ShadowMap>(device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, CASCADE_COUNT);
	cascadeSplits = {0.05f, 0.2f, 1.0f};

	shadowConstants.cascadeCount = CASCADE_COUNT;
	shadowConstants.shadowMapSize = static_cast<float>(SHADOW_MAP_SIZE);
	shadowConstants.pcfKernelSize = 1.0f;
	shadowConstants.bias = 0.001f;
}

void LightManager::Shutdown()
{
	shadowMap.reset();
}

void LightManager::SetDirectionalLight(const LightData::DirectionalLight& dirLight)
{
	directionalLight = dirLight;
	directionalLight.direction.Normalize();
}

const LightData::DirectionalLight& LightManager::GetDirectionalLight() const
{
	return directionalLight;
}

void LightManager::RegisterPointLight(std::shared_ptr<PointLight> light)
{
	if (!light) {
		return;
	}

	// Проверяем, нет ли уже такого света
	for (auto& entry : activeLights) {
		if (auto existing = entry.component.lock()) {
			if (existing.get() == light.get()) {
				return;
			}
		}
	}

	activeLights.push_back({light, 0.0f});
}

void LightManager::UnregisterPointLight(const PointLight* light)
{
	if (!light) {
		return;
	}

	activeLights.erase(
		std::remove_if(
			activeLights.begin(),
			activeLights.end(),
			[light](const LightEntry& entry) {
				auto ptr = entry.component.lock();
				return !ptr || ptr.get() == light;
			}
		),
		activeLights.end()
	);
}

void LightManager::PrepareLights(PerFrameConstants& constants, const Vector3& cameraPosition)
{
	// Очищаем expired указатели и собираем валидные данные
	cachedLights.clear();

	for (auto it = activeLights.begin(); it != activeLights.end();) {
		auto light = it->component.lock();
		if (!light || !light->lightEnabled) {
			it = activeLights.erase(it);
			continue;
		}

		auto data = light->GetLightData();
		it->distanceToCamera = Vector3::DistanceSquared(data.position, cameraPosition);

		cachedLights.push_back(data);
		++it;
	}

	// Сортируем по расстоянию до камеры (ближайшие первыми)
	std::sort(
		cachedLights.begin(),
		cachedLights.end(),
		[&cameraPosition](const LightData::PointLight& a, const LightData::PointLight& b) {
			float distA = Vector3::DistanceSquared(a.position, cameraPosition);
			float distB = Vector3::DistanceSquared(b.position, cameraPosition);
			return distA < distB;
		}
	);

	// Заполняем константный буфер (максимум MAX_POINT_LIGHTS)
	int count = static_cast<int>(std::min<size_t>(cachedLights.size(), MAX_POINT_LIGHTS));
	constants.pointLightCount = count;

	for (int i = 0; i < count; ++i) {
		constants.pointLights[i] = cachedLights[i];
	}

	// Обнуляем оставшиеся слоты (чтобы не было мусора из прошлого кадра)
	for (int i = count; i < MAX_POINT_LIGHTS; ++i) {
		constants.pointLights[i] = LightData::PointLight{};
	}
}

std::vector<Matrix> LightManager::CalculateCascadeMatrices(
	const Vector3& lightDir, const Matrix& cameraView, float fovRadians, float aspect, float nearPlane, float farPlane
)
{
	std::vector<Matrix> matrices;
	float prevSplit = nearPlane;

	for (int i = 0; i < CASCADE_COUNT; ++i) {
		float split = cascadeSplits[i];
		float splitNear = prevSplit;
		float splitFar = nearPlane + (farPlane - nearPlane) * split;

		Matrix subProj = Matrix::CreatePerspectiveFieldOfView(fovRadians, aspect, splitNear, splitFar);
		Matrix invSub = (cameraView * subProj).Invert();

		Vector3 frustumCorners[8];
		for (int j = 0; j < 8; ++j) {
			float x = (j & 1) ? 1.0f : -1.0f;
			float y = (j & 2) ? 1.0f : -1.0f;
			float z = (j & 4) ? 1.0f : 0.0f;

			Vector4 worldPos = Vector4::Transform(Vector4(x, y, z, 1.0f), invSub);
			frustumCorners[j] = Vector3(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w);
		}

		Vector3 center = Vector3::Zero;
		for (int j = 0; j < 8; ++j) {
			center += frustumCorners[j];
		}
		center /= 8.0f;

		float radius = 0.0f;
		for (int j = 0; j < 8; ++j) {
			float dist = Vector3::Distance(frustumCorners[j], center);
			if (dist > radius) {
				radius = dist;
			}
		}

		float texelSize = (radius * 2.0f) / SHADOW_MAP_SIZE;
		Vector3 lightPos = center - lightDir * radius * 2.0f;

		Matrix lightView = Matrix::CreateLookAt(lightPos, center, Vector3::Up);
		Vector3 centerLightView = Vector3::Transform(center, lightView);

		centerLightView.x = std::floor(centerLightView.x / texelSize) * texelSize;
		centerLightView.y = std::floor(centerLightView.y / texelSize) * texelSize;

		Matrix invLightView = lightView.Invert();
		center = Vector3::Transform(centerLightView, invLightView);

		lightPos = center - lightDir * radius * 2.0f;
		lightView = Matrix::CreateLookAt(lightPos, center, Vector3::Up);

		float dist = radius * 2.0f;
		Matrix lightProj = Matrix::CreateOrthographic(radius * 2.0f, radius * 2.0f, dist - radius, dist + radius);

		matrices.push_back(lightView * lightProj);
		shadowConstants.cascades[i].viewProj = matrices.back().Transpose();
		shadowConstants.cascades[i].splitDistance = splitFar;

		prevSplit = splitFar;
	}

	return matrices;
}

void LightManager::RenderShadowCascades(
	ID3D11DeviceContext* ctx, GameContext* gameCtx, ShaderManager* shaders,
	const std::vector<std::shared_ptr<GameComponent>>& sceneObjects, const Vector3& lightDir, const Matrix& cameraView,
	float fovRadians, float aspect, float nearPlane, float farPlane
)
{
	if (!shadowMap) {
		return;
	}

	auto matrices = CalculateCascadeMatrices(lightDir, cameraView, fovRadians, aspect, nearPlane, farPlane);

	ComPtr<ID3D11RenderTargetView> prevRTV;
	ComPtr<ID3D11DepthStencilView> prevDSV;
	ctx->OMGetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.GetAddressOf());

	D3D11_VIEWPORT prevViewport;
	UINT numViewports = 1;
	ctx->RSGetViewports(&numViewports, &prevViewport);

	shaders->ApplyShadow();

	for (int i = 0; i < CASCADE_COUNT; ++i) {
		shadowMap->Clear(ctx, i);
		shadowMap->BeginCascade(ctx, i);

		for (auto& component : sceneObjects) {
			if (!component->GetParent() && component->isActive) {
				std::deque<std::shared_ptr<GameComponent>> queue;
				queue.push_back(component);

				while (!queue.empty()) {
					auto current = queue.front();
					queue.pop_front();

					if (current->isActive && current->GetMesh()) {
						Matrix world = current->GetWorldMatrix();
						shaders->UpdateShadowConstants(world, matrices[i]);
						current->Draw();
					}

					for (auto& child : current->GetChildren()) {
						queue.push_back(child);
					}
				}
			}
		}

		shadowMap->EndCascade(ctx);
	}

	ctx->OMSetRenderTargets(1, prevRTV.GetAddressOf(), prevDSV.Get());
	ctx->RSSetViewports(1, &prevViewport);
}

void LightManager::BindShadowMap(ID3D11DeviceContext* ctx, int slot)
{
	if (shadowMap) {
		auto* srv = shadowMap->GetSRV();
		auto* sampler = shadowMap->GetSampler();
		ctx->PSSetShaderResources(slot, 1, &srv);
		ctx->PSSetSamplers(slot, 1, &sampler);
	}
}

LightData::ShadowConstants LightManager::GetShadowConstants() const
{
	return shadowConstants;
}
