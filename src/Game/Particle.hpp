#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct ParticleConstants {
	Matrix viewProj;

	Vector3 cameraRight;
	float pad0;

	Vector3 cameraUp;
	float pad1;

	Vector2 screenSize;
	float pad2[2];
};
