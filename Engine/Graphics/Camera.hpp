#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>

namespace Engine {

// Логика камеры вынесена в ECS компоненты (Components.h)
// Этот файл можно использовать для вспомогательных функций

using namespace DirectX::SimpleMath;

inline Matrix CalculateViewMatrix(const Vector3& position, const Vector3& target, const Vector3& up)
{
	return Matrix::CreateLookAt(position, target, up);
}

inline Matrix CalculateProjectionMatrix(float fovDegrees, float aspect, float nearPlane, float farPlane)
{
	return Matrix::CreatePerspectiveFieldOfView(fovDegrees * (3.14159f / 180.0f), aspect, nearPlane, farPlane);
}

}  // namespace Engine
