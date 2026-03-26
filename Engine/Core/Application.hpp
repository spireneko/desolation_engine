#pragma once

#include <SDL3/SDL.h>
#include <flecs.h>
#include <memory>

#include "../ECS/Systems.hpp"
#include "Input.hpp"
#include "Renderer.hpp"
#include "Time.hpp"
#include "Window.hpp"

namespace Engine {

class Application {
   public:
	Application() = default;
	~Application() = default;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	bool Initialize(const char* title, int width, int height)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
			return false;
		}

		m_window = std::make_unique<Window>();
		if (!m_window->Create(title, width, height)) {
			return false;
		}

		m_renderer = std::make_unique<Renderer>();
		if (!m_renderer->Initialize(m_window->GetHandle(), width, height)) {
			return false;
		}

		m_input = std::make_unique<Input>();
		m_time = std::make_unique<Time>();

		InitializeECS();

		return true;
	}

	void Run()
	{
		m_isRunning = true;

		while (m_isRunning) {
			m_time->Update();
			float deltaTime = m_time->GetDeltaTime();

			ProcessEvents();

			if (!m_isRunning) {
				break;
			}

			m_input->Update();

			Update(deltaTime);
			Render();

			m_input->EndFrame();
		}
	}

	void Shutdown()
	{
		m_world.reset();
		m_renderer.reset();
		m_window.reset();
		SDL_Quit();
	}

	flecs::world& GetWorld() { return *m_world; }

	Renderer& GetRenderer() { return *m_renderer; }

	Input& GetInput() { return *m_input; }

   private:
	void InitializeECS()
	{
		m_world = std::make_unique<flecs::world>();

		// Регистрация систем
		Systems::RegisterMovementSystem(*m_world);
		Systems::RegisterRotationSystem(*m_world);
		Systems::RegisterCameraControlSystem(*m_world, *m_input);
		Systems::RegisterCameraMovementSystem(*m_world, *m_input);
		Systems::RegisterRenderSystem(*m_world, *m_renderer);
	}

	void ProcessEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					m_isRunning = false;
					break;
				case SDL_EVENT_KEY_DOWN:
				case SDL_EVENT_KEY_UP:
				case SDL_EVENT_MOUSE_MOTION:
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
				case SDL_EVENT_MOUSE_BUTTON_UP:
					m_input->HandleEvent(event);
					break;
				case SDL_EVENT_WINDOW_RESIZED:
					m_renderer->Resize(event.window.data1, event.window.data2);
					break;
			}
		}
	}

	void Update(float deltaTime)
	{
		// Установка delta time для систем
		m_world->set<Components::DeltaTime>({deltaTime});
	}

	void Render()
	{
		m_renderer->BeginFrame();

		// Прогресс ECS
		m_world->progress(m_time->GetDeltaTime());

		m_renderer->EndFrame();
	}

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<Input> m_input;
	std::unique_ptr<Time> m_time;
	std::unique_ptr<flecs::world> m_world;
	bool m_isRunning = false;
};

}  // namespace Engine
