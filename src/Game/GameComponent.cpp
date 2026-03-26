#include "GameComponent.hpp"

GameComponent::GameComponent(GameContext* ctx) : gameContext(ctx) {}

void GameComponent::SetVelocity(const Vector3& vel)
{
	velocity = vel;
	velocity.Normalize();
}

void GameComponent::SetAngularVelocity(const Vector3& angVel)
{
	angularVelocity = angVel;
	angularVelocity.Normalize();
}

Matrix GameComponent::GetWorldMatrix() const
{
	return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
}

void GameComponent::Draw()
{
	if (mesh) {
		mesh->Draw(gameContext->GetGraphicsContext());
	}
}

void GameComponent::Update(float deltaTime)
{
	position += velocity * speed * deltaTime;

	Vector3 angularDelta = angularVelocity * angularSpeed * deltaTime;
	Quaternion deltaRot = Quaternion::CreateFromYawPitchRoll(angularDelta.y, angularDelta.x, angularDelta.z);
	rotation = Quaternion::Concatenate(rotation, deltaRot);
}
