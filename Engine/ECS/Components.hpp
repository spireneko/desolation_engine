#pragma once

#include <SimpleMath.h>

namespace Engine::Components {

using namespace DirectX::SimpleMath;

// === Трансформация ===
struct Position {
	Vector3 value = Vector3::Zero;
};

struct Rotation {
	Quaternion value = Quaternion::Identity;
};

struct Scale {
	Vector3 value = Vector3::One;
};

// === Движение ===
struct Velocity {
	Vector3 value = Vector3::Zero;
};

struct Speed {
	float value = 5.0f;
};

struct RotationSpeed {
	Vector3 value = Vector3::Zero;	// градусы в секунду по каждой оси
};

struct AngularVelocity {
	Vector3 value = Vector3::Zero;
};

// === Ввод для управления ===
struct PlayerControl {
	float moveSpeed = 5.0f;
	float rotationSpeed = 60.0f;  // градусы в секунду
};

struct CameraControl {
	float moveSpeed = 10.0f;
	float mouseSensitivity = 0.1f;
};

// === Камера ===
struct Camera {
	float fov = 60.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;
	bool isActive = true;
};

struct CameraTransform {
	Vector3 position;
	Vector3 forward = Vector3::Forward;
	Vector3 up = Vector3::Up;
	Vector3 right = Vector3::Right;
	float yaw = 0.0f;
	float pitch = 0.0f;
};

// === Графика ===
struct MeshRenderer {
	// Тег-компонент, mesh будет получен из ресурсной системы по entity
	// или храниться как shared_ptr в отдельном компоненте
	Vector4 color = Vector4(1, 1, 1, 1);
};

// === Теги ===
struct CubeMesh {};	 // Тег для создания куба

struct Controllable {};	 // Тег для управляемых объектов

// === Время ===
struct DeltaTime {
	float value = 0.0f;
};

}  // namespace Engine::Components
