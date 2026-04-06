#pragma once

#include <memory>
#include <vector>

#include "GameComponent.hpp"

class KatamariBallComponent;

std::vector<std::shared_ptr<GameComponent>> CreateKatamariGame(
	GameContext* ctx, std::shared_ptr<KatamariBallComponent>& outBall
);
