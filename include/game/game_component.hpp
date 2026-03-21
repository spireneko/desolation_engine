#pragma once

#include "gameable.hpp"

class GameComponent {
   public:
	virtual ~GameComponent() = default;
	virtual void Init(Gameable* game) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void Shutdown() = 0;
};
