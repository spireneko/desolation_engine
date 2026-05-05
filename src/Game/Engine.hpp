#pragma once

#include <SDL3/SDL.h>
#include <deque>
#include <iostream>
#include <memory>
#include <vector>

#include "Colors.hpp"
#include "GameComponent.hpp"
#include "GameContext.hpp"
#include "Graphics.hpp"
#include "Mesh.hpp"
#include "RenderingSystem.hpp"
#include "ShaderManager.hpp"

class Engine : public GameContext {
   public:
	Engine() = default;
	virtual ~Engine() = default;

	bool Initialize(const char* title, int width, int height);
	void Run();
	void Shutdown();

	ID3D11DeviceContext* GetGraphicsContext() override;
	ID3D11Device* GetGraphicsDevice() override;
	LightManager* GetLightManager() override;
	InputManager* GetInputManager() override;
	ShaderManager* GetShaderManager() override;
	Graphics* GetGraphics() override;

   private:
	void ProcessEvents();
	void Update(float deltaTime);
	void UpdateComponent(const std::shared_ptr<GameComponent>& component, float deltaTime);
	void Render();

	SDL_Window* window = nullptr;
	bool running = false;

	std::vector<std::shared_ptr<GameComponent>> gameComponents;
	std::shared_ptr<CameraComponent> currentCamera;

	std::unique_ptr<Graphics> graphics;
	std::unique_ptr<ShaderManager> shaders;
	std::unique_ptr<LightManager> lightManager;
	std::unique_ptr<RenderingSystem> renderingSystem;
	std::unique_ptr<InputManager> inputManager;

	int width, height;
};
