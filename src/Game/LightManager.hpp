#pragma once

#include <memory>

#include "Light.hpp"
#include "PerFrameConstants.hpp"

class PointLight;

class LightManager {
   public:
	static constexpr int MAX_POINT_LIGHTS = 16;

	LightManager() = default;

	void RegisterPointLight(std::shared_ptr<PointLight> light);
	void UnregisterPointLight(const PointLight* light);

	void PrepareLights(PerFrameConstants& constants, const Vector3& cameraPosition);

	size_t GetActiveLightCount() const { return activeLights.size(); }

   private:
	struct LightEntry {
		std::weak_ptr<PointLight> component;
		float distanceToCamera = 0.0f;
	};

	std::vector<LightEntry> activeLights;
	std::vector<LightData::PointLight> cachedLights;
};
