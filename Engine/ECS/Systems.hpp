#pragma once

#include <SimpleMath.h>
#include <flecs.h>
#include <algorithm>

#include "../Core/Input.hpp"
#include "../Core/Renderer.hpp"
#include "../Graphics/Mesh.hpp"
#include "Components.hpp"

namespace Engine::Systems {

using namespace DirectX::SimpleMath;

// === Система движения ===
inline void RegisterMovementSystem(flecs::world& world)
{
	world.system<Components::Position, const Components::Velocity>().each(
		[](flecs::iter& it, size_t i, Components::Position& pos, const Components::Velocity& vel) {
			auto dt = it.world().get<Components::DeltaTime>();
			float deltaTime = dt.value;

			pos.value += vel.value * deltaTime;
		}
	);
}

// === Система вращения ===
inline void RegisterRotationSystem(flecs::world& world)
{
	world.system<Components::Rotation, const Components::AngularVelocity>().each(
		[](flecs::iter& it, size_t i, Components::Rotation& rot, const Components::AngularVelocity& angVel) {
			auto dt = it.world().get<Components::DeltaTime>();
			float deltaTime = dt.value;

			Quaternion deltaRot = Quaternion::CreateFromYawPitchRoll(
				angVel.value.y * deltaTime * (3.14159f / 180.0f),
				angVel.value.x * deltaTime * (3.14159f / 180.0f),
				angVel.value.z * deltaTime * (3.14159f / 180.0f)
			);

			rot.value = rot.value * deltaRot;
			rot.value.Normalize();
		}
	);
}

// === Система управления камерой мышью ===
inline void RegisterCameraControlSystem(flecs::world& world, Input& input)
{
	world.system<Components::CameraTransform, const Components::CameraControl>().each(
		[&input](
			flecs::iter& it, size_t i, Components::CameraTransform& cam, const Components::CameraControl& control
		) {
			Vector2 mouseDelta = input.GetMouseDelta();

			cam.yaw += mouseDelta.x * control.mouseSensitivity * 0.01f;
			cam.pitch -= mouseDelta.y * control.mouseSensitivity * 0.01f;

			// Ограничение pitch
			cam.pitch = std::clamp(cam.pitch, -89.0f, 89.0f);

			// Пересчет векторов направления
			float yawRad = cam.yaw * (3.14159f / 180.0f);
			float pitchRad = cam.pitch * (3.14159f / 180.0f);

			cam.forward.x = cosf(pitchRad) * cosf(yawRad);
			cam.forward.y = sinf(pitchRad);
			cam.forward.z = cosf(pitchRad) * sinf(yawRad);
			cam.forward.Normalize();

			cam.right = cam.forward.Cross(Vector3::Up);
			cam.right.Normalize();

			cam.up = cam.right.Cross(cam.forward);
			cam.up.Normalize();
		}
	);
}

// === Система движения камеры (WASD) ===
inline void RegisterCameraMovementSystem(flecs::world& world, Input& input)
{
	world.system<Components::CameraTransform, const Components::CameraControl>().each(
		[&input](
			flecs::iter& it, size_t i, Components::CameraTransform& cam, const Components::CameraControl& control
		) {
			auto dt = it.world().get<Components::DeltaTime>();
			float deltaTime = dt.value;

			Vector3 movement = Vector3::Zero;

			if (input.IsKeyDown(SDLK_W)) {
				movement += cam.forward;
			}
			if (input.IsKeyDown(SDLK_S)) {
				movement -= cam.forward;
			}
			if (input.IsKeyDown(SDLK_A)) {
				movement -= cam.right;
			}
			if (input.IsKeyDown(SDLK_D)) {
				movement += cam.right;
			}
			if (input.IsKeyDown(SDLK_Q)) {
				movement -= Vector3::Up;
			}
			if (input.IsKeyDown(SDLK_E)) {
				movement += Vector3::Up;
			}

			if (movement.LengthSquared() > 0.001f) {
				movement.Normalize();
				cam.position += movement * control.moveSpeed * deltaTime;
			}
		}
	);
}

// === Система управления объектом (кубом) ===
inline void RegisterObjectControlSystem(flecs::world& world, Input& input)
{
	world
		.system<
			Components::Velocity,
			Components::AngularVelocity,
			const Components::PlayerControl,
			const Components::Controllable>()
		.each([&input](
				  flecs::iter& it,
				  size_t i,
				  Components::Velocity& vel,
				  Components::AngularVelocity& angVel,
				  const Components::PlayerControl& control,
				  const Components::Controllable&
			  ) {
			auto dt = it.world().get<Components::DeltaTime>();
			float deltaTime = dt.value;

			// Сброс скоростей
			vel.value = Vector3::Zero;
			angVel.value = Vector3::Zero;

			// Линейное движение (IJKL для куба, чтобы не конфликтовать с камерой)
			Vector3 moveInput = Vector3::Zero;
			if (input.IsKeyDown(SDLK_I)) {
				moveInput.z += 1;
			}
			if (input.IsKeyDown(SDLK_K)) {
				moveInput.z -= 1;
			}
			if (input.IsKeyDown(SDLK_J)) {
				moveInput.x -= 1;
			}
			if (input.IsKeyDown(SDLK_L)) {
				moveInput.x += 1;
			}
			if (input.IsKeyDown(SDLK_U)) {
				moveInput.y += 1;
			}
			if (input.IsKeyDown(SDLK_O)) {
				moveInput.y -= 1;
			}

			if (moveInput.LengthSquared() > 0.001f) {
				moveInput.Normalize();
				vel.value = moveInput * control.moveSpeed;
			}

			// Вращение (стрелки)
			if (input.IsKeyDown(SDLK_UP)) {
				angVel.value.x -= control.rotationSpeed;
			}
			if (input.IsKeyDown(SDLK_DOWN)) {
				angVel.value.x += control.rotationSpeed;
			}
			if (input.IsKeyDown(SDLK_LEFT)) {
				angVel.value.y -= control.rotationSpeed;
			}
			if (input.IsKeyDown(SDLK_RIGHT)) {
				angVel.value.y += control.rotationSpeed;
			}
		});
}

// === Система рендеринга ===
inline void RegisterRenderSystem(flecs::world& world, Renderer& renderer)
{
	// Сначала обновляем матрицы камеры
	world.system<Components::Camera, const Components::CameraTransform>().each(
		[&renderer](
			flecs::iter& it, size_t i, const Components::Camera& cam, const Components::CameraTransform& camTransform
		) {
			if (!cam.isActive) {
				return;
			}

			Matrix view = Matrix::CreateLookAt(
				camTransform.position, camTransform.position + camTransform.forward, camTransform.up
			);

			int width, height;
			// Получаем размер из рендерера или используем дефолт
			float aspect = 1280.0f / 720.0f;  // TODO: получать реальный aspect

			Matrix projection = Matrix::CreatePerspectiveFieldOfView(
				cam.fov * (3.14159f / 180.0f), aspect, cam.nearPlane, cam.farPlane
			);

			renderer.SetCameraConstants(view, projection);
		}
	);

	// Рендерим меши
	world
		.system<
			const Components::Position,
			const Components::Rotation,
			const Components::Scale,
			const Components::MeshRenderer>()
		.each([&renderer](
				  flecs::iter& it,
				  size_t i,
				  const Components::Position& pos,
				  const Components::Rotation& rot,
				  const Components::Scale& scale,
				  const Components::MeshRenderer& mesh
			  ) {
			// Создаем меш куба на лету (в реальном проекте используйте кэш)
			static std::unique_ptr<Mesh> cubeMesh;
			if (!cubeMesh) {
				cubeMesh = std::make_unique<Mesh>();
				cubeMesh->CreateCube(renderer.GetDevice());
			}

			Matrix world = Matrix::CreateScale(scale.value) * Matrix::CreateFromQuaternion(rot.value) *
						   Matrix::CreateTranslation(pos.value);

			renderer.DrawMesh(*cubeMesh, world);
		});
}

}  // namespace Engine::Systems
