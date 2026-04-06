#include "KatamariBallComponent.hpp"
#include "InputManager.hpp"

KatamariBallComponent::KatamariBallComponent(GameContext* ctx) : GameComponent(ctx)
{
	auto* input = ctx->GetInputManager();
	input->RegisterAction("katamari_forward", {SDLK_W}, {}, InputState::Held);
	input->RegisterAction("katamari_backward", {SDLK_S}, {}, InputState::Held);
	input->RegisterAction("katamari_left", {SDLK_A}, {}, InputState::Held);
	input->RegisterAction("katamari_right", {SDLK_D}, {}, InputState::Held);

	moveSpeed = 10.0;
	SetBoundingRadius(0.5);
}

void KatamariBallComponent::SetWorldObjects(const std::vector<std::shared_ptr<GameComponent>>& objects)
{
	worldObjects.clear();
	for (auto& object : objects) {
		worldObjects.push_back(object);
	}
}

void KatamariBallComponent::SetCamera(OrbitalCamera* camera_)
{
	camera = camera_;
}

void KatamariBallComponent::Update(float deltaTime)
{
	Vector3 moveDir = Vector3::Zero;
	auto* input = gameContext->GetInputManager();

	Vector3 forward = Vector3(0, 0, 1);
	Vector3 right = Vector3(-1, 0, 0);
	if (camera) {
		forward = camera->GetForward();
		right = camera->GetRight();
		forward.y = 0.0f;
		right.y = 0.0f;
		if (forward.LengthSquared() > 0.0f) {
			forward.Normalize();
		}
		if (right.LengthSquared() > 0.0f) {
			right.Normalize();
		}
	}

	if (input->IsActionActive("katamari_forward")) {
		moveDir += forward;
	}
	if (input->IsActionActive("katamari_backward")) {
		moveDir -= forward;
	}
	if (input->IsActionActive("katamari_left")) {
		moveDir -= right;
	}
	if (input->IsActionActive("katamari_right")) {
		moveDir += right;
	}

	if (moveDir.LengthSquared() > 0.0f) {
		moveDir.Normalize();
		float distance = moveSpeed * deltaTime;
		position += moveDir * distance;

		Vector3 spinAxis = Vector3::Up.Cross(moveDir);
		float spinAngle = distance / GetBoundingRadius();
		rotation = rotation * Quaternion::CreateFromAxisAngle(spinAxis, spinAngle);
	}

	TryAttachObjects();

	if (camera) {
		camera->SetTarget(position);
	}

	GameComponent::Update(deltaTime);
}

void KatamariBallComponent::TryAttachObjects()
{
	for (auto& weakObject : worldObjects) {
		auto object = weakObject.lock();
		if (!object) {
			continue;
		}
		if (object->GetParent()) {
			continue;
		}
		Vector3 delta = object->position - position;
		float distance = delta.Length();
		float threshold = GetBoundingRadius() + object->GetBoundingRadius();
		if (distance <= threshold) {
			AttachObject(object);
		}
	}
}

bool KatamariBallComponent::AttachObject(const std::shared_ptr<GameComponent>& object)
{
	object->SetVelocity(Vector3::Zero);
	object->speed = 0.0f;
	object->scale = object->scale / scale;

	Vector3 direction = object->position - position;
	if (direction.LengthSquared() > 0.0f) {
		direction.Normalize();
	} else {
		direction = Vector3(0, 1, 0);
	}

	object->position = direction * GetBoundingRadius() / scale;
	// object->position = direction * (GetBoundingRadius() + object->GetBoundingRadius());
	AddChild(object);
	return true;
}
