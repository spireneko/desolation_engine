#include "KatamariGame.hpp"
#include "KatamariBallComponent.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

static std::shared_ptr<GameComponent> CreateTexturedObj(
	GameContext* ctx, const std::string& path, const std::string& baseDir, const Vector3& position,
	const Vector3& scale, float radius, std::shared_ptr<Texture> texture
)
{
	auto object = std::make_shared<GameComponent>(ctx);
	object->position = position;
	object->scale = scale;
	object->SetBoundingRadius(radius);

	auto mesh = std::make_unique<Mesh>();
	if (!mesh->LoadFromObj(ctx, path, baseDir, std::move(texture))) {
		return nullptr;
	}
	object->SetMesh(std::move(mesh));
	return object;
}

std::vector<std::shared_ptr<GameComponent>> CreateKatamariGame(
	GameContext* ctx, std::shared_ptr<KatamariBallComponent>& outBall
)
{
	std::vector<std::shared_ptr<GameComponent>> components;
	std::vector<std::shared_ptr<GameComponent>> stickables;

	// Ground grid
	auto grid = std::make_shared<GameComponent>(ctx);
	auto gridMesh = std::make_unique<Mesh>();
	gridMesh->CreateGrid(ctx, 120.0f, 24, Colors::LightGray);
	grid->SetMesh(std::move(gridMesh));
	components.push_back(grid);

	auto pickTexture = std::make_shared<Texture>();
	if (!pickTexture->LoadFromFile(ctx, L"assets/textures/Alice guitar pick_BaseColor.png")) {
		pickTexture = Texture::CreateWhite(ctx);
	}

	// Ball
	outBall = std::make_shared<KatamariBallComponent>(ctx);
	outBall->scale = Vector3(2.5, 2.5, 2.5);
	outBall->position.y = outBall->GetBoundingRadius();

	auto outBallMesh = std::make_unique<Mesh>();
	outBallMesh->CreateSphere(ctx, 24, 24);
	outBallMesh->SetTexture(pickTexture);
	outBall->SetMesh(std::move(outBallMesh));
	components.push_back(outBall);

	// Objects loaded from OBJ
	std::string assetsFolder = "assets/models/";

	auto crateTexture = std::make_shared<Texture>();
	if (!crateTexture->LoadFromFile(ctx, L"assets/textures/wood.jpg")) {
		crateTexture = Texture::CreateWhite(ctx);
	}
	auto crate = CreateTexturedObj(
		ctx,
		"assets/models/cube.obj",
		assetsFolder,
		Vector3(5.0f, 0.5f, 6.0f),
		Vector3(1.0f, 1.0f, 1.0f),
		0.5f,
		crateTexture
	);
	if (crate) {
		components.push_back(crate);
		stickables.push_back(crate);
	}

	auto stoneTexture = std::make_shared<Texture>();
	if (!stoneTexture->LoadFromFile(ctx, L"assets/textures/brick.jpg")) {
		stoneTexture = Texture::CreateWhite(ctx);
	}
	auto stone = CreateTexturedObj(
		ctx,
		"assets/models/pyramid.obj",
		assetsFolder,
		Vector3(-4.0f, 0.5f, 7.0f),
		Vector3(1.2f, 1.2f, 1.2f),
		0.1f,
		stoneTexture
	);
	if (stone) {
		components.push_back(stone);
		stickables.push_back(stone);
	}

	// Ball
	auto ball = std::make_shared<GameComponent>(ctx);
	ball->scale = Vector3(2, 2, 2);
	ball->position = Vector3(3.0, ball->GetBoundingRadius(), 10.0);

	auto ballMesh = std::make_unique<Mesh>();
	ballMesh->CreateSphere(ctx, 12, 12);
	ballMesh->SetTexture(stoneTexture);
	ball->SetMesh(std::move(ballMesh));
	if (ball) {
		components.push_back(ball);
		stickables.push_back(ball);
	}

	// Picks
	auto pick = CreateTexturedObj(
		ctx,
		"assets/models/Alice_guitar_pick.obj",
		assetsFolder,
		Vector3(3.0f, 0.5f, -4.0f),
		Vector3(0.4f, 0.4f, 0.4f),
		0.4f,
		pickTexture
	);
	auto pickMat = pick->GetMaterial();
	pickMat.shininess = 1 << 8;
	pick->SetMaterial(pickMat);
	if (pick) {
		components.push_back(pick);
		stickables.push_back(pick);
	}

	// Chair
	auto chairTexture = std::make_shared<Texture>();
	if (!chairTexture->LoadFromFile(ctx, L"assets/textures/old_chair_mat_bcolor.png")) {
		chairTexture = Texture::CreateWhite(ctx);
	}
	auto chair = CreateTexturedObj(
		ctx,
		"assets/models/old_chair.obj",
		assetsFolder,
		Vector3(-6.0f, 0.5f, -9.0f),
		Vector3(0.3f, 0.3f, 0.3f),
		0.4f,
		chairTexture
	);
	if (chair) {
		components.push_back(chair);
		stickables.push_back(chair);
	}

	// Sword
	auto swordTexture = std::make_shared<Texture>();
	if (!swordTexture->LoadFromFile(ctx, L"assets/textures/practice_sword_mat_bcolor.png")) {
		swordTexture = Texture::CreateWhite(ctx);
	}
	auto sword = CreateTexturedObj(
		ctx,
		"assets/models/practice_sword.obj",
		assetsFolder,
		Vector3(-8.0f, 0.5f, -4.0f),
		Vector3(0.5, 0.5, 0.5),
		1,
		swordTexture
	);
	if (sword) {
		components.push_back(sword);
		stickables.push_back(sword);
	}

	outBall->SetWorldObjects(stickables);
	outBall->SetCamera(nullptr);

	return components;
}
