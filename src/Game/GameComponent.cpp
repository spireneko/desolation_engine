#include "GameComponent.hpp"

GameComponent::GameComponent(GameContext* ctx) : gameContext(ctx) {}

void GameComponent::SetVelocity(const Vector3& vel)
{
	velocity = vel;
	if (velocity.LengthSquared() > 0.0f) {
		velocity.Normalize();
	}
}

void GameComponent::SetAngularVelocity(const Vector3& angVel)
{
	angularVelocity = angVel;
	if (angularVelocity.LengthSquared() > 0.0f) {
		angularVelocity.Normalize();
	}
}

void GameComponent::SetRotation(const Quaternion& rot)
{
	rotation = rot;
}

Quaternion GameComponent::GetRotation() const
{
	return rotation;
}

void GameComponent::SetBoundingRadius(float radius)
{
	boundingRadius = radius;
}

float GameComponent::GetBoundingRadius() const
{
	return boundingRadius * std::max(scale.x, std::max(scale.y, scale.z));
}

void GameComponent::AddChild(std::shared_ptr<GameComponent> child)
{
	if (child->parent) {
		child->parent->RemoveChild(child.get());
	}
	child->parent = this;
	children.push_back(child);
}

void GameComponent::RemoveChild(GameComponent* child)
{
	auto it = std::find_if(children.begin(), children.end(), [child](const std::shared_ptr<GameComponent>& c) {
		return c.get() == child;
	});
	if (it != children.end()) {
		(*it)->parent = nullptr;
		children.erase(it);
	}
}

GameComponent* GameComponent::GetParent() const
{
	return parent;
}

const std::vector<std::shared_ptr<GameComponent>>& GameComponent::GetChildren() const
{
	return children;
}

void GameComponent::SetMesh(std::unique_ptr<Mesh>&& m)
{
	mesh = std::move(m);
}

Mesh* GameComponent::GetMesh() const
{
	return mesh.get();
}

void GameComponent::SetMaterial(const Material& mat)
{
	material = mat;
}

const Material& GameComponent::GetMaterial() const
{
	return material;
}

Matrix GameComponent::GetWorldMatrix() const
{
	Matrix localMatrix =
		Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
	if (parent) {
		return localMatrix * parent->GetWorldMatrix();
	}
	return localMatrix;
}

void GameComponent::Draw()
{
	if (mesh) {
		mesh->Draw(gameContext);
	}
}

void GameComponent::Update(float deltaTime)
{
	position += velocity * speed * deltaTime;

	Vector3 angularDelta = angularVelocity * angularSpeed * deltaTime;
	Quaternion deltaRot = Quaternion::CreateFromYawPitchRoll(angularDelta.y, angularDelta.x, angularDelta.z);
	rotation = Quaternion::Concatenate(rotation, deltaRot);
}
