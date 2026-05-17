#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

namespace LightData {
struct DirectionalLight {
	Vector3 direction;
	float intensity;

	Vector3 color;
	float padding0;
};

struct PointLight {
	Vector3 position;
	float intensity;

	Vector3 color;
	float range;

	float constant;
	float linear;
	float quadratic;
	float padding0;
};

struct SpotLight {
	Vector3 position;
	float intensity;

	Vector3 color;
	float range;

	Vector3 direction;
	float innerAngle;  // radians

	float outerAngle;  // radians
	float constant;
	float linear;
	float quadratic;

	float pad;
};

struct CascadeData {
	Matrix viewProj;

	float splitDistance;
	float pad[3];
};

struct ShadowConstants {
	CascadeData cascades[4];

	int cascadeCount;
	float shadowMapSize;
	float pcfKernelSize;
	float bias;
};
}  // namespace LightData

struct LightingConstants {
	Vector3 cameraPosition;
	int cascadeCount;
	LightData::DirectionalLight dirLight;
	Matrix invViewProj;
	LightData::ShadowConstants shadowData;
	Vector2 screenSize;
	Vector2 pad;
};

struct PointLightConstants {
	LightData::PointLight pointLight;

	Matrix invViewProj;

	Vector3 cameraPosition;
	float pad;

	Matrix worldViewProj;
	Vector2 screenSize;
	Vector2 pad2;
};

struct SpotLightConstants {
	Vector3 position;
	float intensity;

	Vector3 color;
	float range;

	Vector3 direction;
	float innerAngle;

	float outerAngle;
	float constant;
	float linearAttenuation;
	float quadratic;

	Matrix invViewProj;

	Vector3 cameraPosition;
	float pad2;

	Matrix worldViewProj;

	Vector2 screenSize;
	Vector2 pad3;
};
