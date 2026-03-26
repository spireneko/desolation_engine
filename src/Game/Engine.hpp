#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <vector>

#include "Camera.hpp"
#include "GameComponent.hpp"
#include "GameContext.hpp"
#include "Graphics.hpp"
#include "Mesh.hpp"
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

   private:
	void ProcessEvents();
	void Update(float deltaTime);
	void Render();

	SDL_Window* window = nullptr;
	bool running = false;

	std::vector<std::unique_ptr<GameComponent>> gameComponents;

	std::unique_ptr<Graphics> graphics;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Mesh> cube;
	std::unique_ptr<ShaderManager> shaders;

	int width, height;
};
