#pragma once

#include <SDL3/SDL.h>

namespace Engine {

class Time {
   public:
	Time() { m_lastTime = SDL_GetTicks() / 1000.0f; }

	void Update()
	{
		float currentTime = SDL_GetTicks() / 1000.0f;
		m_deltaTime = currentTime - m_lastTime;
		m_lastTime = currentTime;
		m_totalTime += m_deltaTime;
	}

	float GetDeltaTime() const { return m_deltaTime; }

	float GetTotalTime() const { return m_totalTime; }

   private:
	float m_lastTime = 0.0f;
	float m_deltaTime = 0.0f;
	float m_totalTime = 0.0f;
};

}  // namespace Engine
