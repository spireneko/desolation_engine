#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct DirectionalLight {
	Vector3 direction;
	float intensity;

	Vector3 color;
	float padding0;
};
