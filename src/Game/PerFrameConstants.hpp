#pragma once

#include <SimpleMath.h>

#include "Light.hpp"
#include "Material.hpp"

using namespace DirectX::SimpleMath;

struct PerFrameConstants {
	Matrix world;

	Matrix view;

	Matrix projection;

	Vector3 cameraPosition;
	int pointLightCount;

	LightData::DirectionalLight dirLight;

	LightData::PointLight pointLights[16];

	Material material;
};
