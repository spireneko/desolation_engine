#pragma once

#include <memory>
#include <vector>

#include "Light.hpp"
#include "PerFrameConstants.hpp"
#include "ShadowMap.hpp"

class PointLight;
class SpotLight;
class ShaderManager;
class GameContext;
class GameComponent;

class LightManager {
   public:
	static constexpr int MAX_POINT_LIGHTS = 16;
	static constexpr int CASCADE_COUNT = 3;
	static constexpr int SHADOW_MAP_SIZE = 1024;
	static constexpr float SHADOW_DISTANCE = 100.0;

	LightManager();
	~LightManager() = default;

	void Initialize(ID3D11Device* device);
	void Shutdown();

	void SetDirectionalLight(const LightData::DirectionalLight& dirLight);
	const LightData::DirectionalLight& GetDirectionalLight() const;

	void RegisterPointLight(std::shared_ptr<PointLight> light);
	void UnregisterPointLight(const PointLight* light);

	void RegisterSpotLight(std::shared_ptr<SpotLight> light);
	void UnregisterSpotLight(const SpotLight* light);

	void PrepareLights(PerFrameConstants& constants, const Vector3& cameraPosition);

	void RenderShadowCascades(
		ID3D11DeviceContext* ctx, GameContext* gameCtx, ShaderManager* shaders,
		const std::vector<std::shared_ptr<GameComponent>>& sceneObjects, const Vector3& lightDir,
		const Matrix& cameraView, float fovRadians, float aspect, float nearPlane, float farPlane
	);

	void BindShadowMap(ID3D11DeviceContext* ctx, int slot);
	LightData::ShadowConstants GetShadowConstants() const;

	size_t GetActiveLightCount() const { return activeLights.size(); }

	size_t GetActiveSpotLightCount() const { return activeSpotLights.size(); }

	const std::vector<LightData::PointLight>& GetCachedLights() const { return cachedLights; }

	const std::vector<LightData::SpotLight>& GetCachedSpotLights() const { return cachedSpotLights; }

   private:
	struct LightEntry {
		std::weak_ptr<PointLight> component;
		float distanceToCamera = 0.0f;
	};

	struct SpotLightEntry {
		std::weak_ptr<SpotLight> component;
		float distanceToCamera = 0.0f;
	};

	LightData::DirectionalLight directionalLight;

	std::vector<LightEntry> activeLights;
	std::vector<LightData::PointLight> cachedLights;

	std::vector<SpotLightEntry> activeSpotLights;
	std::vector<LightData::SpotLight> cachedSpotLights;

	std::unique_ptr<ShadowMap> shadowMap;
	LightData::ShadowConstants shadowConstants;
	std::vector<float> cascadeSplits;

	std::vector<Matrix> CalculateCascadeMatrices(
		const Vector3& lightDir, const Matrix& cameraView, float fovRadians, float aspect, float nearPlane,
		float farPlane
	);
};
