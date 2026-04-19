#pragma once

#include "GameComponent.hpp"
#include "Light.hpp"

class PointLight : public GameComponent {
   public:
   static std::shared_ptr<PointLight> Create(
        GameContext* ctx,
        const LightData::PointLight& data,
        bool autoRegister = true
    );

    ~PointLight() override;

	void SetLightData(const LightData::PointLight& data);
	LightData::PointLight GetLightData() const;

	bool lightEnabled = true;

   private:
	PointLight(GameContext* ctx, const LightData::PointLight& lightData);

	LightData::PointLight localLightData;
};
