#pragma once

#include <memory>
#include <vector>

#include "../Game/Cube.hpp"
#include "../Game/GameComponent.hpp"

std::vector<std::shared_ptr<GameComponent>> CreatePlanetsGame(GameContext* ctx);
