#include "KatamariBallComponent.hpp"

#include "InputManager.hpp"
#include "PointLight.hpp"

KatamariBallComponent::KatamariBallComponent(GameContext* ctx) : GameComponent(ctx)
{
	auto* input = ctx->GetInputManager();
	input->RegisterAction("katamari_forward", {SDLK_W}, {}, InputState::Held);
	input->RegisterAction("katamari_backward", {SDLK_S}, {}, InputState::Held);
	input->RegisterAction("katamari_left", {SDLK_A}, {}, InputState::Held);
	input->RegisterAction("katamari_right", {SDLK_D}, {}, InputState::Held);
	input->RegisterAction("katamari_shoot", {SDLK_SPACE}, {}, InputState::Pressed);

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

	if (input->IsActionActive("katamari_shoot")) {
		DetachLastObject();
	}

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
	object->SetAngularVelocity(Vector3::Zero);
	object->angularSpeed = 0.0f;

	Vector3 worldOffset = object->position - position;

	Quaternion invRotation;
	rotation.Inverse(invRotation);

	Vector3 localOffset = Vector3::Transform(worldOffset, invRotation);
	object->position = localOffset / scale;

	object->SetRotation(invRotation * object->GetRotation());

	object->scale = object->scale / scale;

	AddChild(object);
	return true;
}

void KatamariBallComponent::DetachLastObject()
{
	if (children.empty()) {
		return;
	}

	auto lastObject = children.back();

	LightData::PointLight lightData;
	lightData.position = Vector3::Zero;
	lightData.intensity = 3.0f;
	lightData.color = Vector3(1.0f, 0.7f, 0.3f);
	lightData.range = 15.0f;
	lightData.constant = 1.0f;
	lightData.linear = 0.14f;
	lightData.quadratic = 0.07f;

	auto pointLight = PointLight::Create(gameContext, lightData);
	lastObject->AddChild(pointLight);

	Matrix worldMatrix = lastObject->GetWorldMatrix();
	Vector3 worldPos;
	Quaternion worldRot;
	Vector3 worldScale;
	worldMatrix.Decompose(worldScale, worldRot, worldPos);

	RemoveChild(lastObject.get());

	lastObject->position = worldPos;
	lastObject->SetRotation(worldRot);
	lastObject->scale = worldScale;

	Vector3 shootDirection = camera->GetForward();
	float shootSpeed = 15.0f;
	lastObject->speed = shootSpeed;
	lastObject->SetVelocity(shootDirection);
}
