#include "OrbitalCamera.hpp"

OrbitalCamera::OrbitalCamera(
	GameContext* ctx, float initialDistance, Vector3 target_, float minDistance, float maxDistance
)
	: CameraComponent(ctx),
	  target(target_),
	  distance(initialDistance),
	  minDistance(minDistance),
	  maxDistance(maxDistance)
{}

void OrbitalCamera::Update(float deltaTime)
{
	float deltaX, deltaY;
	gameContext->GetInputManager()->GetMouseDelta(deltaX, deltaY);

	if (deltaX != 0.0f || deltaY != 0.0f) {
		yaw -= deltaX * mouseSensitivity;
		pitch += deltaY * mouseSensitivity;
		const float pitchLimit = 1.45f;
		pitch = std::clamp(pitch, -pitchLimit, pitchLimit);
	}

	// Zoom to target
	if (moveForward) {
		distance = std::max(minDistance, distance - zoomSpeed * deltaTime);
	}
	if (moveBackward) {
		distance = std::min(maxDistance, distance + zoomSpeed * deltaTime);
	}

	Matrix RotMat = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0.0);
	position = Vector3::Transform(Vector3(0, 0, -distance), RotMat) + target;

	moveForward = moveBackward = false;
}

Matrix OrbitalCamera::GetViewMatrix() const
{
	return Matrix::CreateLookAt(position, target, Vector3::Up);
}

Vector3 OrbitalCamera::GetForward() const
{
	Matrix yawRotation = Matrix::CreateRotationY(yaw);
	return Vector3::Transform(Vector3(0, 0, 1), yawRotation);
}

Vector3 OrbitalCamera::GetRight() const
{
	Matrix yawRotation = Matrix::CreateRotationY(yaw);
	return Vector3::Transform(Vector3(-1, 0, 0), yawRotation);
}

void OrbitalCamera::SetTarget(const Vector3& newTarget)
{
	target = newTarget;
}
