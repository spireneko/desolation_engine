#pragma once

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

struct Material {
	Vector3 ambient;
	float padding0;

	Vector3 diffuse;
	float padding1;

	Vector3 specular;
	float shininess;
};
