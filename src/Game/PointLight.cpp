#include "PointLight.hpp"

std::shared_ptr<PointLight> PointLight::Create(
    GameContext* ctx,
    const LightData::PointLight& data,
    bool autoRegister)
{
    auto ptr = std::shared_ptr<PointLight>(new PointLight(ctx, data));

    if (autoRegister) {
        ctx->GetLightManager()->RegisterPointLight(ptr);
    }

    return ptr;
}

PointLight::PointLight(GameContext* ctx, const LightData::PointLight& lightData)
	: GameComponent(ctx), localLightData(lightData)
{
}

PointLight::~PointLight()
{
    gameContext->GetLightManager()->UnregisterPointLight(this);
}

LightData::PointLight PointLight::GetLightData() const
{
    LightData::PointLight result = localLightData;

    Matrix world = GetWorldMatrix();
	Vector3 scale;
	Quaternion rotation;
	world.Decompose(scale, rotation, result.position);

    return result;
}

void PointLight::SetLightData(const LightData::PointLight& data)
{
    localLightData = data;
}
