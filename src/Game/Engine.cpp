#include "Engine.hpp"

#include <iostream>

#include "Colors.hpp"

Engine::Engine() = default;
Engine::~Engine() = default;

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

	// Инициализация Graphics
	graphics = std::make_unique<Graphics>();
	if (!graphics->Initialize(window, width, height)) {
		std::cerr << "Failed to initialize DirectX 11" << std::endl;
		return false;
	}

	// Камера
	camera = std::make_unique<Camera>();
	camera->SetPosition(Vector3(0, -5, -5));
	camera->SetTarget(Vector3(0, 0, 0));
	camera->SetPerspective(90.0f, static_cast<float>(width) / height, 0.1f, 100.0f);

	// Шейдеры
	shaders = std::make_unique<ShaderManager>();
	if (!shaders->Initialize(graphics->GetDevice())) {
		std::cerr << "Failed to initialize shaders" << std::endl;
		return false;
	}

	// Куб
	cube = std::make_unique<Mesh>();
	if (!cube->CreateCube(graphics->GetDevice())) {
		std::cerr << "Failed to create cube mesh" << std::endl;
		return false;
	}

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
		switch (e.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				graphics->Resize(e.window.data1, e.window.data2);
				camera->SetPerspective(60.0f, static_cast<float>(e.window.data1) / e.window.data2, 0.1f, 100.0f);
				break;
			case SDL_EVENT_KEY_DOWN:
				if (e.key.key == SDLK_ESCAPE) {
					running = false;
				}
				break;
		}
	}
}

void Engine::Update(float deltaTime)
{
	rotationY += deltaTime * 1.0f;	// 1 радиан в секунду
}

void Engine::Render()
{
	graphics->BeginFrame(Colors::CornflowerBlue);

	// Матрицы
	Matrix world = Matrix::CreateRotationY(rotationY) * Matrix::CreateRotationX(0.3f);
	Matrix view = camera->GetViewMatrix();
	Matrix proj = camera->GetProjectionMatrix();

	// Обновление констант
	struct Matrices {
		Matrix world;
		Matrix view;
		Matrix projection;
	} mats;

	mats.world = world.Transpose();	 // HLSL ожидает column-major
	mats.view = view.Transpose();
	mats.projection = proj.Transpose();

	shaders->Apply(graphics->GetContext());
	shaders->UpdateConstants(graphics->GetContext(), &mats, sizeof(mats));

	// Рисуем куб
	cube->Draw(graphics->GetContext());

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
