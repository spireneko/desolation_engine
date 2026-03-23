#pragma once

#include <d3d11.h>
#include <vector>

class GameComponent;
class DisplayWin32;
class InputDevice;

class Gameable {
   public:
	virtual ID3D11Device* GetDevice() const = 0;
	virtual ID3D11DeviceContext* GetContext() const = 0;
	virtual DisplayWin32* GetDisplay() = 0;
	virtual InputDevice& GetInput() = 0;
	virtual const std::vector<std::unique_ptr<GameComponent>>& GetComponents() const = 0;

	virtual int GetScorePlayer1() const = 0;
	virtual int GetScorePlayer2() const = 0;
	virtual void AddScorePlayer1() = 0;
	virtual void AddScorePlayer2() = 0;
	virtual void ResetScores() = 0;

	virtual void Restart() = 0;

	virtual ~Gameable() = default;
};
