#pragma once

#include "gameable.hpp"

class GameComponent {
   public:
	explicit GameComponent(Gameable* game) : m_game(game) {}

	virtual ~GameComponent() = default;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual void Shutdown() = 0;
	virtual void Restart() = 0;

   protected:
	Gameable* m_game;
};
