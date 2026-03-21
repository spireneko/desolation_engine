#pragma once

#include <d3d11.h>

#include "display.hpp"

class Gameable {
   public:
	virtual ID3D11Device* GetDevice() const = 0;
	virtual ID3D11DeviceContext* GetContext() const = 0;
	virtual DisplayWin32* GetDisplay() = 0;
	virtual InputDevice& GetInput() = 0;

	virtual ~Gameable() = default;
};
