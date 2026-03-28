#include "Engine.hpp"

#include <deque>
#include <iostream>

#include "../GameSet/Planets.hpp"
#include "Colors.hpp"

bool Engine::Initialize(const char* title, int w, int h)
{
	width = w;
	height = h;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// Включаем relative mouse mode для вращения камеры
	SDL_SetWindowRelativeMouseMode(window, true);

	// Инициализация Graphics
	graphics = std::make_unique<Graphics>();
	if (!graphics->Initialize(window, width, height)) {
		std::cerr << "Failed to initialize DirectX 11" << std::endl;
		return false;
	}

	// Шейдеры
	shaders = std::make_unique<ShaderManager>(this);

	// Input Manager
	inputManager = std::make_unique<InputManager>();

	// Непосредственно игра, состоящая из набора объектов
	gameComponents = CreatePlanetsGame(this);

	camera = std::make_shared<CameraComponent>(this);
	camera->SetAspectRatio(static_cast<float>(width) / height);

	gameComponents.push_back(camera);

	running = true;
	return true;
}

void Engine::Run()
{
	Uint64 lastTime = SDL_GetTicks();

	while (running) {
		Uint64 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0f;
		lastTime = currentTime;

		ProcessEvents();
		Update(deltaTime);
		Render();
	}
}

void Engine::ProcessEvents()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		inputManager->ProcessEvent(e);

		switch (e.type) {
			case SDL_EVENT_QUIT: {
				running = false;
				break;
			}
			case SDL_EVENT_WINDOW_RESIZED: {
				graphics->Resize(e.window.data1, e.window.data2);
				camera->SetAspectRatio(static_cast<float>(e.window.data1) / e.window.data2);
				break;
			}
			case SDL_EVENT_KEY_DOWN: {
				if (e.key.key == SDLK_ESCAPE) {
					running = false;
				}
				break;
				// case SDL_EVENT_MOUSE_BUTTON_DOWN:
				// 	if (e.button.button == SDL_BUTTON_RIGHT) {
				// 		SDL_SetRelativeMouseMode(SDL_TRUE);
				// 	}
				// 	break;
				// case SDL_EVENT_MOUSE_BUTTON_UP:
				// 	if (e.button.button == SDL_BUTTON_RIGHT) {
				// 		SDL_SetRelativeMouseMode(SDL_FALSE);
				// 	}
				// 	break;
			}
		}
	}
}

void Engine::Update(float deltaTime)
{
	inputManager->Update();

	for (auto& component : gameComponents) {
		if (!component->GetParent()) {	// Обновляем только корневые компоненты
			UpdateComponent(component, deltaTime);
		}
	}

	inputManager->EndFrame();
}

void Engine::UpdateComponent(const std::shared_ptr<GameComponent>& component, float deltaTime)
{
	std::deque<std::shared_ptr<GameComponent>> queue;
	queue.push_back(component);

	while (!queue.empty()) {
		auto current = queue.front();
		queue.pop_front();

		current->Update(deltaTime);
		for (auto& child : current->GetChildren()) {
			queue.push_back(child);
		}
	}
}

void Engine::DrawComponent(const std::shared_ptr<GameComponent>& component, const Matrix& view, const Matrix& proj)
{
	std::deque<std::shared_ptr<GameComponent>> queue;
	queue.push_back(component);

	struct Matrices {
		Matrix world;
		Matrix view;
		Matrix projection;
	} mats;

	mats.view = view.Transpose();
	mats.projection = proj.Transpose();

	while (!queue.empty()) {
		auto current = queue.front();
		queue.pop_front();

		Matrix world = current->GetWorldMatrix();
		mats.world = world.Transpose();

		shaders->Apply();
		shaders->UpdateConstants(&mats);

		current->Draw();
		for (auto& child : current->GetChildren()) {
			queue.push_back(child);
		}
	}
}

void Engine::Render()
{
	graphics->BeginFrame(Colors::DarkGray);

	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();

	for (auto& component : gameComponents) {
		if (!component->GetParent()) {
			DrawComponent(component, view, proj);
		}
	}

	graphics->EndFrame();
}

void Engine::Shutdown()
{
	cube.reset();
	shaders.reset();
	camera.reset();
	graphics.reset();

	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	SDL_Quit();
}

ID3D11DeviceContext* Engine::GetGraphicsContext()
{
	return graphics->GetContext();
}

ID3D11Device* Engine::GetGraphicsDevice()
{
	return graphics->GetDevice();
}

InputManager* Engine::GetInputManager()
{
	return inputManager.get();
}
