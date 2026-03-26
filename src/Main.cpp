#include <SDL3/SDL_main.h>
#include <iostream>

#include "Game/Engine.hpp"

int main(int argc, char* argv[])
{
	Engine engine;

	if (!engine.Initialize("SDL3 + DirectX11 + SimpleMath Cube", 1280, 720)) {
		std::cerr << "Engine initialization failed!" << std::endl;
		return -1;
	}

	engine.Run();
	engine.Shutdown();

	return 0;
}
