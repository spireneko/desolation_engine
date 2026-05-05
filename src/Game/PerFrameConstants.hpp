#pragma once

#include <SimpleMath.h>

#include "Light.hpp"
#include "Material.hpp"

using namespace DirectX::SimpleMath;

struct PerFrameConstants {
	Matrix view;

	Matrix projection;

	Vector3 cameraPosition;
	int pointLightCount;

	LightData::DirectionalLight dirLight;

	LightData::PointLight pointLights[16];

	LightData::ShadowConstants shadowData;
};

struct PerObjectConstants {
	Matrix world;

	Material material;
};
