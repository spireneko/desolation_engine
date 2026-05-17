#pragma once

#include "GameComponent.hpp"
#include "Light.hpp"

class SpotLight : public GameComponent {
   public:
	static std::shared_ptr<SpotLight> Create(
		GameContext* ctx, const LightData::SpotLight& data, bool autoRegister = true
	);

	~SpotLight() override;

	void SetLightData(const LightData::SpotLight& data);
	LightData::SpotLight GetLightData() const;

	bool lightEnabled = true;

   private:
	SpotLight(GameContext* ctx, const LightData::SpotLight& lightData);

	LightData::SpotLight localLightData;
};
