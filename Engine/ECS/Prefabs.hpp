#pragma once

#include <flecs.h>

#include "Components.hpp"

namespace Engine::Prefabs {

// === Создание камеры ===
inline flecs::entity CreateCamera(
	flecs::world& world, const DirectX::SimpleMath::Vector3& position = {0, 0, -10}, float yaw = 0.0f,
	float pitch = 0.0f
)
{
	return world.entity()
		.set<Components::Camera>({})
		.set<Components::CameraTransform>(
			{position,
			 DirectX::SimpleMath::Vector3::Forward,
			 DirectX::SimpleMath::Vector3::Up,
			 DirectX::SimpleMath::Vector3::Right,
			 yaw,
			 pitch}
		)
		.set<Components::CameraControl>({
			10.0f,	// moveSpeed
			0.1f	// mouseSensitivity
		});
}

// === Создание куба ===
inline flecs::entity CreateCube(
	flecs::world& world, const DirectX::SimpleMath::Vector3& position = {0, 0, 0},
	const DirectX::SimpleMath::Vector3& scale = {1, 1, 1}, const DirectX::SimpleMath::Vector4& color = {1, 1, 1, 1}
)
{
	return world.entity()
		.set<Components::Position>({position})
		.set<Components::Rotation>({DirectX::SimpleMath::Quaternion::Identity})
		.set<Components::Scale>({scale})
		.set<Components::Velocity>({DirectX::SimpleMath::Vector3::Zero})
		.set<Components::AngularVelocity>({DirectX::SimpleMath::Vector3::Zero})
		.set<Components::MeshRenderer>({color});
}

// === Создание управляемого куба ===
inline flecs::entity CreateControllableCube(
	flecs::world& world, const DirectX::SimpleMath::Vector3& position = {0, 0, 0},
	const DirectX::SimpleMath::Vector3& scale = {1, 1, 1}, const DirectX::SimpleMath::Vector4& color = {1, 0, 0, 1}
)
{
	return CreateCube(world, position, scale, color)
		.add<Components::Controllable>()
		.set<Components::PlayerControl>({
			5.0f,  // moveSpeed
			90.0f  // rotationSpeed (deg/sec)
		});
}

// === Создание вращающегося куба (автоматически) ===
inline flecs::entity CreateRotatingCube(
	flecs::world& world, const DirectX::SimpleMath::Vector3& position = {0, 0, 0},
	const DirectX::SimpleMath::Vector3& rotationSpeed = {0, 30, 0},
	const DirectX::SimpleMath::Vector3& scale = {1, 1, 1}, const DirectX::SimpleMath::Vector4& color = {0, 1, 0, 1}
)
{
	return CreateCube(world, position, scale, color).set<Components::AngularVelocity>({rotationSpeed});
}

// === Создание движущегося куба (с velocity) ===
inline flecs::entity CreateMovingCube(
	flecs::world& world, const DirectX::SimpleMath::Vector3& position = {0, 0, 0},
	const DirectX::SimpleMath::Vector3& velocity = {1, 0, 0}, const DirectX::SimpleMath::Vector3& scale = {1, 1, 1},
	const DirectX::SimpleMath::Vector4& color = {0, 0, 1, 1}
)
{
	return CreateCube(world, position, scale, color).set<Components::Velocity>({velocity});
}

}  // namespace Engine::Prefabs
