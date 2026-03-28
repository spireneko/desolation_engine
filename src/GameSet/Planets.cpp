#include "Planets.hpp"

std::vector<std::shared_ptr<GameComponent>> CreatePlanetsGame(GameContext* ctx)
{
	std::vector<std::shared_ptr<GameComponent>> components;

	auto solarSystem = std::make_shared<GameComponent>(ctx);
	components.push_back(solarSystem);

	auto sun = std::make_shared<GameComponent>(ctx);
	components.push_back(sun);
	sun->scale = Vector3(3, 3, 3);
	sun->SetAngularVelocity(Vector3(0, 1, 0));
	sun->angularSpeed = 0.3;
	auto cubeMesh = std::make_unique<Mesh>();
	cubeMesh->CreateCube(ctx);
	sun->SetMesh(std::move(cubeMesh));
	solarSystem->AddChild(sun);

	auto sunOrbit1 = std::make_shared<GameComponent>(ctx);
	components.push_back(sunOrbit1);
	sunOrbit1->SetAngularVelocity(Vector3(0, 1, 0));
	sunOrbit1->angularSpeed = 1.0;
	solarSystem->AddChild(sunOrbit1);

	auto sunOrbit2 = std::make_shared<GameComponent>(ctx);
	components.push_back(sunOrbit2);
	sunOrbit2->SetAngularVelocity(Vector3(0, 1.0, 1.5));
	sunOrbit2->angularSpeed = 5.0;
	solarSystem->AddChild(sunOrbit2);

	auto sunOrbit3 = std::make_shared<GameComponent>(ctx);
	components.push_back(sunOrbit3);
	sunOrbit3->SetAngularVelocity(Vector3(1, 1, 0));
	sunOrbit3->angularSpeed = 0.1;
	solarSystem->AddChild(sunOrbit3);

	auto planet1 = std::make_shared<GameComponent>(ctx);
	components.push_back(planet1);
	planet1->position = Vector3(5, 0, 0);
	planet1->SetAngularVelocity(Vector3(0.3, 1, 0));
	planet1->angularSpeed = 0.7;
	auto cubeMesh1 = std::make_unique<Mesh>();
	cubeMesh1->CreateCube(ctx);
	planet1->SetMesh(std::move(cubeMesh1));
	sunOrbit1->AddChild(planet1);

	auto planet2 = std::make_shared<GameComponent>(ctx);
	components.push_back(planet2);
	planet2->position = Vector3(-10, 0, 3);
	planet2->SetAngularVelocity(Vector3(0, 1, 0));
	planet2->angularSpeed = 0.4;
	auto cubeMesh2 = std::make_unique<Mesh>();
	cubeMesh2->CreateCube(ctx);
	planet2->SetMesh(std::move(cubeMesh2));
	sunOrbit1->AddChild(planet2);

	auto planet3 = std::make_shared<GameComponent>(ctx);
	components.push_back(planet3);
	planet3->position = Vector3(0, 14, 21);
	planet3->SetAngularVelocity(Vector3(1, 1, 1));
	planet3->angularSpeed = 2;
	auto cubeMesh3 = std::make_unique<Mesh>();
	cubeMesh3->CreateCube(ctx);
	planet3->SetMesh(std::move(cubeMesh3));
	sunOrbit2->AddChild(planet3);

	return components;
}
