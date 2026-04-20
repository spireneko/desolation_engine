#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "GameContext.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

class GameComponent {
   public:
	Vector3 position = Vector3(0, 0, 0);
	Vector3 scale = Vector3(1, 1, 1);
	float speed = 0.0;
	float angularSpeed = 0.0;

	bool isActive = true;

	GameComponent(GameContext* ctx);
	virtual ~GameComponent() = default;

	void SetVelocity(const Vector3& vel);

	void SetAngularVelocity(const Vector3& angVel);
	void SetRotation(const Quaternion& rot);
	Quaternion GetRotation() const;
	void Rotate(float yawDegrees, float pitchDegrees, float rollDegrees);

	void SetBoundingRadius(float radius);
	float GetBoundingRadius() const;

	void AddChild(std::shared_ptr<GameComponent> child);
	void RemoveChild(GameComponent* child);
	GameComponent* GetParent() const;
	const std::vector<std::shared_ptr<GameComponent>>& GetChildren() const;

	void SetMesh(std::unique_ptr<Mesh>&& m);
	Mesh* GetMesh() const;

	void SetMaterial(const Material& mat);
	const Material& GetMaterial() const;

	Matrix GetWorldMatrix() const;

	virtual void Draw();
	virtual void Update(float deltaTime);

   protected:
	GameContext* gameContext;
	std::unique_ptr<Mesh> mesh;

	Vector3 velocity = Vector3(0, 0, 0);
	Quaternion rotation = Quaternion::Identity;
	Vector3 angularVelocity = Vector3(0, 0, 0);
	float boundingRadius = 0.5f;

	Material material = {
		Vector3(0.2, 0.2, 0.2),	 // ambient
		0.0,					 // padding
		Vector3(0.5, 0.5, 0.5),	 // diffuse
		0.0,					 // padding
		Vector3(0.7, 0.7, 0.7),	 // specular
		32.0,					 // shininess
	};

	GameComponent* parent = nullptr;
	std::vector<std::shared_ptr<GameComponent>> children;
};
