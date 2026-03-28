#include "CameraComponent.hpp"

CameraComponent::CameraComponent(GameContext* ctx) : GameComponent(ctx)
{
	auto* input = ctx->GetInputManager();

	// Регистрация действий (коллбеки устанавливают флаги)
	input->RegisterAction("cam_forward", {SDLK_W}, {}, InputState::Held, [this]() { moveForward = true; });
	input->RegisterAction("cam_backward", {SDLK_S}, {}, InputState::Held, [this]() { moveBackward = true; });
	input->RegisterAction("cam_left", {SDLK_A}, {}, InputState::Held, [this]() { moveLeft = true; });
	input->RegisterAction("cam_right", {SDLK_D}, {}, InputState::Held, [this]() { moveRight = true; });

	speed = 15.0;

	position = Vector3(0, 0, -5);
}

void CameraComponent::Update(float deltaTime)
{
	// Вращение мышью
	float deltaX, deltaY;
	gameContext->GetInputManager()->GetMouseDelta(deltaX, deltaY);

	if (deltaX != 0.0f || deltaY != 0.0f) {
		yaw -= deltaX * mouseSensitivity;
		pitch += deltaY * mouseSensitivity;

		// Ограничение pitch
		const float limit = 1.55f;
		pitch = std::max(-limit, std::min(limit, pitch));

		// Обновляем quaternion для совместимости с GameComponent
		rotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0);
	}

	// Вычисляем направления
	Vector3 right = Vector3::Transform(Vector3(-1, 0, 0), Matrix::CreateRotationY(yaw));
	Vector3 worldForward = Vector3::Transform(Vector3(0, 0, 1), rotation);

	// Движение относительно направления взгляда
	Vector3 moveDir(0, 0, 0);
	if (moveForward) {
		moveDir += worldForward;
	}
	if (moveBackward) {
		moveDir -= worldForward;
	}
	if (moveRight) {
		moveDir += right;
	}
	if (moveLeft) {
		moveDir -= right;
	}

	if (moveDir.LengthSquared() > 0) {
		moveDir.Normalize();
		position += moveDir * speed * deltaTime;
	}

	// Сброс флагов после использования
	moveForward = moveBackward = moveLeft = moveRight = false;
}

void CameraComponent::SetPerspective(float fovDegrees_, float aspectRatio_, float nearPlane_, float farPlane_)
{
	fovDegrees = fovDegrees_;
	aspectRatio = aspectRatio_;
	nearPlane = nearPlane_;
	farPlane = farPlane_;
}

void CameraComponent::GetPerspective(float* fovDegrees_, float* aspectRatio_, float* nearPlane_, float* farPlane_) const
{
	if (fovDegrees_) {
		*fovDegrees_ = fovDegrees;
	}
	if (aspectRatio_) {
		*aspectRatio_ = aspectRatio;
	}
	if (nearPlane_) {
		*nearPlane_ = nearPlane;
	}
	if (farPlane_) {
		*farPlane_ = farPlane;
	}
}

void CameraComponent::SetFOV(float fovDegrees)
{
	this->fovDegrees = fovDegrees;
}

void CameraComponent::SetAspectRatio(float aspectRatio)
{
	this->aspectRatio = aspectRatio;
}

Matrix CameraComponent::GetViewMatrix() const
{
	// Создаем view matrix из position и rotation
	Vector3 target = position + Vector3::Transform(Vector3(0, 0, 1), rotation);
	Vector3 up = Vector3::Transform(Vector3(0, 1, 0), rotation);
	return Matrix::CreateLookAt(position, target, up);
}

Matrix CameraComponent::GetProjectionMatrix() const
{
	return Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(fovDegrees), aspectRatio, nearPlane, farPlane
	);
}
