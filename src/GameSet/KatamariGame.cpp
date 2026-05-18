#include "KatamariGame.hpp"

#include "Game/KatamariBallComponent.hpp"
#include "Game/Mesh.hpp"
#include "Game/ParticleEmitterComponent.hpp"
#include "Game/PointLight.hpp"
#include "Game/SpotLight.hpp"
#include "Game/Texture.hpp"

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

std::pair<std::vector<std::shared_ptr<GameComponent>>, std::shared_ptr<OrbitalCamera>> CreateKatamariGame(
	GameContext* ctx
)
{
	std::vector<std::shared_ptr<GameComponent>> components;
	std::vector<std::shared_ptr<GameComponent>> stickables;

	// Ground grid
	// auto grid = std::make_shared<GameComponent>(ctx);
	// auto gridMesh = std::make_unique<Mesh>();
	// gridMesh->CreateGrid(ctx, 120.0f, 24, Colors::LightGray);
	// grid->SetMesh(std::move(gridMesh));
	// components.push_back(grid);

	auto camera = std::make_shared<OrbitalCamera>(ctx, 20, Vector3(0, 0, 0));
	components.push_back(camera);

	// Floor
	auto grassTexture = std::make_shared<Texture>();
	if (!grassTexture->LoadFromFile(ctx, L"assets/textures/minegrass.jpg")) {
		grassTexture = Texture::CreateWhite(ctx);
	}

	auto floor = std::make_shared<GameComponent>(ctx);
	auto floorMesh = std::make_unique<Mesh>();
	floorMesh->CreatePlane(ctx);
	floorMesh->SetTexture(grassTexture);
	floor->SetMesh(std::move(floorMesh));
	floor->scale = Vector3(120.0f, 1.0f, 120.0f);
	auto floorMat = floor->GetMaterial();
	floorMat.shininess = 64;
	floor->SetMaterial(floorMat);
	components.push_back(floor);

	auto miniFloor = std::make_shared<GameComponent>(ctx);
	auto miniFloorMesh = std::make_unique<Mesh>();
	miniFloorMesh->CreatePlane(ctx);
	miniFloorMesh->SetTexture(grassTexture);
	miniFloor->SetMesh(std::move(miniFloorMesh));
	miniFloor->position.y = 3.5f;
	miniFloor->scale = Vector3(5, 1, 5);
	miniFloor->Rotate(0, 180, 0);
	components.push_back(miniFloor);

	auto pickTexture = std::make_shared<Texture>();
	if (!pickTexture->LoadFromFile(ctx, L"assets/textures/Alice guitar pick_BaseColor.png")) {
		pickTexture = Texture::CreateWhite(ctx);
	}

	// Ball
	auto outBall = std::make_shared<KatamariBallComponent>(ctx);
	outBall->SetCamera(camera.get());
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
		Vector3(5.0, 1.0, 6.0),
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
		Vector3(-4.0, 1.2, 7.0),
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
	pickMat.specular = Vector3(0.8f, 0.8f, 0.8f);
	pick->SetMaterial(pickMat);
	pick->Rotate(90, 0, 0);
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

	// Spark emitter for sword
	auto sparkTexture = Texture::CreateWhite(ctx);
	auto sparkEmitter = std::make_shared<ParticleEmitterComponent>(ctx);
	EmitterSettings sparkSettings;
	sparkSettings.maxParticles = 200;
	sparkSettings.emissionRate = 50;  // manual burst
	sparkSettings.lifetimeMin = 0.4f;
	sparkSettings.lifetimeMax = 0.7f;
	sparkSettings.velocityMin = Vector3(-5, 2, -5);
	sparkSettings.velocityMax = Vector3(5, 8, 5);
	sparkSettings.startSize = 0.1f;
	sparkSettings.endSize = 0.0f;
	sparkSettings.startColor = Colors::Yellow;
	sparkSettings.endColor = Colors::Red;
	sparkSettings.startAlpha = 1.0f;
	sparkSettings.endAlpha = 0.0f;
	sparkSettings.gravity = Vector3(0, -15.0f, 0);
	sparkSettings.blendMode = BlendMode::Additive;
	sparkSettings.texture = sparkTexture;

	sparkEmitter->SetSettings(sparkSettings);
	sword->AddChild(sparkEmitter);

	LightData::PointLight lightData;
	lightData.position = Vector3::Zero;
	lightData.intensity = 3.0f;
	lightData.color = Vector3(1.0f, 0.7f, 0.3f);
	lightData.range = 15.0f;
	lightData.constant = 1.0f;
	lightData.linear = 0.14f;
	lightData.quadratic = 0.07f;

	auto pointLight = PointLight::Create(ctx, lightData);
	pointLight->position = Vector3(0.0f, 2.0f, 5.0f);
	components.push_back(pointLight);

	LightData::SpotLight spotLightData;
	spotLightData.intensity = 3.0f;
	spotLightData.color = Vector3(1.0f, 0.7f, 0.3f);
	spotLightData.range = 25.0f;
	spotLightData.constant = 1.0f;
	spotLightData.linear = 0.14f;
	spotLightData.quadratic = 0.07f;
	spotLightData.innerAngle = DirectX::XMConvertToRadians(30.0f);
	spotLightData.outerAngle = DirectX::XMConvertToRadians(45.0f);
	spotLightData.direction = Vector3(0.0f, -1.0f, 0.0f);

	auto spotLight = SpotLight::Create(ctx, spotLightData);
	spotLight->position = Vector3(-5.0f, 8.0f, -5.0f);
	components.push_back(spotLight);

	outBall->SetWorldObjects(stickables);

	return std::make_pair(components, camera);
}
