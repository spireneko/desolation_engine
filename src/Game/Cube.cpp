#include "Cube.hpp"

Cube::Cube(GameContext* ctx) : GameComponent(ctx)
{
	mesh = std::make_unique<Mesh>();
	mesh->CreateCube(gameContext->GetGraphicsDevice());
}
