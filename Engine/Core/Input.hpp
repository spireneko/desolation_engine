#pragma once

#include <SDL3/SDL.h>
#include <SimpleMath.h>
#include <unordered_map>

namespace Engine {

using namespace DirectX::SimpleMath;

class Input {
   public:
	Input() = default;

	void HandleEvent(const SDL_Event& event)
	{
		switch (event.type) {
			case SDL_EVENT_KEY_DOWN:
				m_keys[event.key.key] = true;
				m_keysPressed[event.key.key] = true;
				break;
			case SDL_EVENT_KEY_UP:
				m_keys[event.key.key] = false;
				break;
			case SDL_EVENT_MOUSE_MOTION:
				m_mouseDeltaX += event.motion.xrel;
				m_mouseDeltaY += event.motion.yrel;
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				m_mouseButtons[event.button.button] = true;
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				m_mouseButtons[event.button.button] = false;
				break;
		}
	}

	void Update()
	{
		// Вызывается в начале кадра
	}

	void EndFrame()
	{
		// Сброс одноразовых состояний
		m_keysPressed.clear();
		m_mouseDeltaX = 0;
		m_mouseDeltaY = 0;
	}

	bool IsKeyDown(SDL_Keycode key) const
	{
		auto it = m_keys.find(key);
		return it != m_keys.end() && it->second;
	}

	bool IsKeyPressed(SDL_Keycode key) const
	{
		auto it = m_keysPressed.find(key);
		return it != m_keysPressed.end() && it->second;
	}

	bool IsMouseButtonDown(Uint8 button) const
	{
		auto it = m_mouseButtons.find(button);
		return it != m_mouseButtons.end() && it->second;
	}

	void GetMouseDelta(float& x, float& y) const
	{
		x = static_cast<float>(m_mouseDeltaX);
		y = static_cast<float>(m_mouseDeltaY);
	}

	Vector2 GetMouseDelta() const
	{
		return Vector2(static_cast<float>(m_mouseDeltaX), static_cast<float>(m_mouseDeltaY));
	}

   private:
	std::unordered_map<SDL_Keycode, bool> m_keys;
	std::unordered_map<SDL_Keycode, bool> m_keysPressed;
	std::unordered_map<Uint8, bool> m_mouseButtons;

	float m_mouseDeltaX = 0;
	float m_mouseDeltaY = 0;
};

}  // namespace Engine
