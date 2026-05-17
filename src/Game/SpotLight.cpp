#include "SpotLight.hpp"

#include "LightManager.hpp"

std::shared_ptr<SpotLight> SpotLight::Create(GameContext* ctx, const LightData::SpotLight& data, bool autoRegister)
{
	auto ptr = std::shared_ptr<SpotLight>(new SpotLight(ctx, data));

	if (autoRegister) {
		ctx->GetLightManager()->RegisterSpotLight(ptr);
	}

	return ptr;
}

SpotLight::SpotLight(GameContext* ctx, const LightData::SpotLight& lightData)
	: GameComponent(ctx), localLightData(lightData)
{}

SpotLight::~SpotLight()
{
	gameContext->GetLightManager()->UnregisterSpotLight(this);
}

LightData::SpotLight SpotLight::GetLightData() const
{
	LightData::SpotLight result = localLightData;

	Matrix world = GetWorldMatrix();
	Vector3 scale;
	Quaternion rotation;
	world.Decompose(scale, rotation, result.position);

	// Rotate direction into world space
	Vector3 localDir = Vector3::Transform(localLightData.direction, rotation);
	localDir.Normalize();
	result.direction = localDir;

	return result;
}

void SpotLight::SetLightData(const LightData::SpotLight& data)
{
	localLightData = data;
}
