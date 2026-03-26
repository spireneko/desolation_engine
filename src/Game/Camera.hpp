#pragma once

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Camera {
   public:
	Camera();

	void SetPosition(const Vector3& pos);
	void SetTarget(const Vector3& target);
	void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);

	Matrix GetViewMatrix() const;
	Matrix GetProjectionMatrix() const;

	void RotateAroundTarget(float deltaX, float deltaY);

   private:
	Vector3 position;
	Vector3 target;
	Vector3 up;
	Matrix projection;
};
