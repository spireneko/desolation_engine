#pragma once

#include <d3d11.h>

#include "InputManager.hpp"
#include "LightManager.hpp"

class GameContext {
   public:
	GameContext() = default;
	virtual ~GameContext() = default;

	virtual ID3D11DeviceContext* GetGraphicsContext() = 0;
	virtual ID3D11Device* GetGraphicsDevice() = 0;
	virtual LightManager* GetLightManager() = 0;
	virtual InputManager* GetInputManager() = 0;
};
