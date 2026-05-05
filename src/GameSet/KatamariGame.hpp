#pragma once

#include <memory>
#include <vector>

#include "Game/GameComponent.hpp"

class KatamariBallComponent;
class OrbitalCamera;

std::pair<std::vector<std::shared_ptr<GameComponent>>, std::shared_ptr<OrbitalCamera>> CreateKatamariGame(
	GameContext* ctx
);
