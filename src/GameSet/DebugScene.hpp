#pragma once

#include <memory>
#include <vector>
#include "Game/GameComponent.hpp"

class GameContext;
class FPSCamera;

// Создаёт отладочную сцену: FPS камера, шар, пол, несколько тестовых объектов
std::pair<std::vector<std::shared_ptr<GameComponent>>, std::shared_ptr<FPSCamera>> CreateDebugScene(GameContext* ctx);
