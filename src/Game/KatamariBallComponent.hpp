#pragma once

#include <vector>
#include "GameComponent.hpp"
#include "OrbitalCamera.hpp"

class KatamariBallComponent : public GameComponent {
   public:
	KatamariBallComponent(GameContext* ctx);

	void SetWorldObjects(const std::vector<std::shared_ptr<GameComponent>>& objects);
	void SetCamera(OrbitalCamera* camera);

	void Update(float deltaTime) override;

   private:
	std::vector<std::weak_ptr<GameComponent>> worldObjects;
	OrbitalCamera* camera = nullptr;
	float moveSpeed = 10.0f;

	void TryAttachObjects();
	bool AttachObject(const std::shared_ptr<GameComponent>& object);
};
