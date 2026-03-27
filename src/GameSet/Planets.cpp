#include "Planets.hpp"

std::vector<std::shared_ptr<GameComponent>> CreatePlanetsGame(GameContext* ctx)
{
	std::vector<std::shared_ptr<GameComponent>> components;

	auto cube = std::make_shared<Cube>(ctx);
	cube->angularSpeed = 1.0;
	cube->SetAngularVelocity(Vector3(0, 1, 0));

	// Дочерний куб, привязанный к позиционированию родителя
	auto childCube = std::make_shared<Cube>(ctx);
	childCube->angularSpeed = 2.0;
	childCube->SetAngularVelocity(Vector3(0, 0, 1));
	childCube->position = Vector3(2, 0, 0);	 // относительно родителя
	childCube->scale = Vector3(0.5f, 0.5f, 0.5f);
	cube->AddChild(childCube);

	components.push_back(cube);

	auto cube2 = std::make_shared<Cube>(ctx);
	cube2->angularSpeed = 2.0;
	cube2->SetAngularVelocity(Vector3(0, 0, 1));
	cube2->position = Vector3(-4, 0, 0);
	cube2->scale = Vector3(1.5, 0.5, 1.5);
	components.push_back(cube2);

	// cube->AddChild(cube2);

	return components;
}
