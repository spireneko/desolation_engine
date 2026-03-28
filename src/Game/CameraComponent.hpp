#pragma once

#include <algorithm>

#include "GameComponent.hpp"

class CameraComponent : public GameComponent {
   public:
	CameraComponent(GameContext* ctx);
	virtual ~CameraComponent() = default;

	void Update(float deltaTime) override;

	void SetPerspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
	void GetPerspective(float* fovDegrees, float* aspectRatio, float* nearPlane, float* farPlane) const;
	void SetFOV(float fovDegrees);
	void SetAspectRatio(float aspectRatio);

	Matrix GetViewMatrix() const;
	Matrix GetProjectionMatrix() const;

   private:
	float yaw = 0.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.0005f;

	float fovDegrees = 60.0;
	float aspectRatio = 16.0 / 9.0;
	float nearPlane = 0.1;
	float farPlane = 10000.0;

	// Флаги движения (устанавливаются коллбеками InputManager)
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
};
