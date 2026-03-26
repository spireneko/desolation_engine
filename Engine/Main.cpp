#include <SDL3/SDL_main.h>

#include "Core/Application.hpp"
#include "ECS/Prefabs.hpp"
#include "ECS/Systems.hpp"

int main(int argc, char* argv[])
{
	Engine::Application app;

	if (!app.Initialize("Desolation Engine", 1280, 720)) {
		return -1;
	}

	auto& world = app.GetWorld();

	// === Создание сущностей ===

	// 1. Камера (управляется WASD + мышь)
	auto camera = Engine::Prefabs::CreateCamera(world, {0, 0, 0}, 0, 0);

	// // 2. Управляемый куб (IJKL для движения, стрелки для вращения)
	// auto playerCube = Engine::Prefabs::CreateControllableCube(
	// 	world,
	// 	{0, 0, 0},				  // position
	// 	{1, 1, 1},				  // scale
	// 	{1.0f, 0.2f, 0.2f, 1.0f}  // red color
	// );

	// // 3. Автоматически вращающийся куб
	// auto rotatingCube = Engine::Prefabs::CreateRotatingCube(
	// 	world,
	// 	{3, 0, 0},				  // position
	// 	{0, 45, 0},				  // rotation speed (deg/sec)
	// 	{0.8f, 0.8f, 0.8f},		  // scale
	// 	{0.2f, 1.0f, 0.2f, 1.0f}  // green color
	// );

	// // 4. Движущийся куб (по оси X)
	// auto movingCube = Engine::Prefabs::CreateMovingCube(
	// 	world,
	// 	{-3, 0, 0},				  // position
	// 	{2, 0, 0},				  // velocity (units/sec)
	// 	{0.6f, 0.6f, 0.6f},		  // scale
	// 	{0.2f, 0.2f, 1.0f, 1.0f}  // blue color
	// );

	// 5. Статичный куб (просто стоит)
	auto staticCube = Engine::Prefabs::CreateCube(
		world,
		{5, 5, 0},				  // position
		{0.5f, 0.5f, 0.5f},		  // scale
		{1.0f, 1.0f, 0.2f, 1.0f}  // yellow color
	);

	// Добавляем систему управления объектом (после создания сущностей)
	Engine::Systems::RegisterObjectControlSystem(world, app.GetInput());

	// Запуск главного цикла
	app.Run();
	app.Shutdown();

	return 0;
}
