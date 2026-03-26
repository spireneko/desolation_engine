#include "Planets.hpp"

std::vector<std::unique_ptr<GameComponent>> CreatePlanetsGame(GameContext* ctx)
{
	std::vector<std::unique_ptr<GameComponent>> components;

	auto cube = std::make_unique<Cube>(ctx);
	cube->angularSpeed = 1.0;
	cube->SetAngularVelocity(Vector3(0, 1, 0));
	components.push_back(std::move(cube));

	auto cube2 = std::make_unique<Cube>(ctx);
	cube2->angularSpeed = 2.0;
	cube2->SetAngularVelocity(Vector3(0, 0, 1));
	cube2->position = Vector3(-4, 0, 0);
	cube2->scale = Vector3(1.5, 0.5, 1.5);
	components.push_back(std::move(cube2));

	return components;
}
