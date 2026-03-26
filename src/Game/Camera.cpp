#include "Camera.hpp"

Camera::Camera() : position(0, 0, -5), target(0, 0, 0), up(0, 1, 0) {}

void Camera::SetPosition(const Vector3& pos)
{
	position = pos;
}

void Camera::SetTarget(const Vector3& t)
{
	target = t;
}

void Camera::SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
{
	projection = Matrix::CreatePerspectiveFieldOfView(fovDegrees * 3.14159f / 180.0f, aspectRatio, nearPlane, farPlane);
}

Matrix Camera::GetViewMatrix() const
{
	return Matrix::CreateLookAt(position, target, up);
}

Matrix Camera::GetProjectionMatrix() const
{
	return projection;
}

void Camera::RotateAroundTarget(float deltaX, float deltaY)
{
	// Простое вращение камеры
	Matrix rotation = Matrix::CreateRotationY(deltaX) * Matrix::CreateRotationX(deltaY);
	Vector3 offset = position - target;
	offset = Vector3::Transform(offset, rotation);
	position = target + offset;
}
