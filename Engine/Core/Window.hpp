#pragma once

#include <SDL3/SDL.h>
#include <windows.h>

namespace Engine {

class Window {
   public:
	Window() = default;

	~Window()
	{
		if (m_window) {
			SDL_DestroyWindow(m_window);
		}
	}

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool Create(const char* title, int width, int height)
	{
		m_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

		if (!m_window) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation failed: %s", SDL_GetError());
			return false;
		}

		// Скрываем курсор для FPS-режима
		SDL_SetWindowRelativeMouseMode(m_window, true);

		return true;
	}

	HWND GetHandle() const
	{
		return static_cast<HWND>(
			SDL_GetPointerProperty(SDL_GetWindowProperties(m_window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)
		);
	}

	SDL_Window* GetSDLWindow() const { return m_window; }

	void GetSize(int& width, int& height) const { SDL_GetWindowSize(m_window, &width, &height); }

   private:
	SDL_Window* m_window = nullptr;
};

}  // namespace Engine
