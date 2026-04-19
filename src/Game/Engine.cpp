#include "Engine.hpp"

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

	lightManager = std::make_unique<LightManager>();

	// Input Manager
	inputManager = std::make_unique<InputManager>();

	// Непосредственно игра, состоящая из набора объектов
	gameComponents = CreateKatamariGame(this, ball);

	fixedCamera = std::make_shared<OrbitalCamera>(this, 20, Vector3(0, 0, 0));
	fixedCamera->SetAspectRatio(static_cast<float>(width) / height);
	fixedCamera->isActive = true;
	if (ball) {
		ball->SetCamera(fixedCamera.get());
	}
	gameComponents.push_back(fixedCamera);

	running = true;
	return true;
}

void Engine::Run()
{
	Uint64 lastTime = SDL_GetTicks();

	while (running) {
		Uint64 currentTime = SDL_GetTicks();
		float deltaTime = (currentTime - lastTime) / 1000.0;
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
				fixedCamera->SetAspectRatio(static_cast<float>(e.window.data1) / e.window.data2);
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

		if (current->isActive) {
			current->Update(deltaTime);
			for (auto& child : current->GetChildren()) {
				queue.push_back(child);
			}
		}
	}
}

void Engine::DrawComponent(const std::shared_ptr<GameComponent>& component, const Matrix& view, const Matrix& proj)
{
	std::deque<std::shared_ptr<GameComponent>> queue;
	queue.push_back(component);

	PerFrameConstants constants;

	constants.view = view.Transpose();
	constants.projection = proj.Transpose();

	constants.cameraPosition = fixedCamera->position;

	constants.dirLight.direction = Vector3(1.3, -1.0, 0.2);
	constants.dirLight.direction.Normalize();
	constants.dirLight.color = Vector3(1.0, 1.0, 1.0);
	constants.dirLight.intensity = 1.0;

	lightManager->PrepareLights(constants, fixedCamera->position);

	while (!queue.empty()) {
		auto current = queue.front();
		queue.pop_front();

		Matrix world = current->GetWorldMatrix();
		constants.world = world.Transpose();
		constants.material = current->GetMaterial();

		shaders->Apply();
		shaders->UpdateConstants(&constants);

		current->Draw();
		for (auto& child : current->GetChildren()) {
			queue.push_back(child);
		}
	}
}

void Engine::Render()
{
	graphics->BeginFrame(Colors::DarkGray);

	Matrix view = fixedCamera->GetViewMatrix();
	Matrix proj = fixedCamera->GetProjectionMatrix();

	for (auto& component : gameComponents) {
		if (!component->GetParent()) {
			DrawComponent(component, view, proj);
		}
	}

	graphics->EndFrame();
}

void Engine::Shutdown()
{
	shaders.reset();
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

LightManager* Engine::GetLightManager()
{
	return lightManager.get();
}

InputManager* Engine::GetInputManager()
{
	return inputManager.get();
}
