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
