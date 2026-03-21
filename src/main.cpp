#include "game/game.hpp"

int main()
{
	try {
		Game game("My3DApp", 800, 800, GetModuleHandle(nullptr));
		game.Run();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
