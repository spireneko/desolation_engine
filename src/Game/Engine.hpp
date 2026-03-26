#pragma once

#include <SDL3/SDL.h>
#include <memory>

#include "Camera.hpp"
#include "Graphics.hpp"
#include "Mesh.hpp"
#include "ShaderManager.hpp"

class Engine {
   public:
	Engine();
	~Engine();

	bool Initialize(const char* title, int width, int height);
	void Run();
	void Shutdown();

   private:
	void ProcessEvents();
	void Update(float deltaTime);
	void Render();

	SDL_Window* window = nullptr;
	bool running = false;

	std::unique_ptr<Graphics> graphics;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Mesh> cube;
	std::unique_ptr<ShaderManager> shaders;

	float rotationY = 0.0f;
	int width, height;
};
