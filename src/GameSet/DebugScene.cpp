#include "DebugScene.hpp"

#include "Game/Colors.hpp"
#include "Game/FPSCamera.hpp"
#include "Game/Mesh.hpp"
#include "Game/Texture.hpp"

static std::shared_ptr<GameComponent> CreateTestObject(
	GameContext* ctx, const Vector3& position, const Vector3& scale, std::shared_ptr<Texture> texture,
	const Vector4& color = Colors::White
)
{
	auto obj = std::make_shared<GameComponent>(ctx);
	obj->position = position;
	obj->scale = scale;
	obj->SetBoundingRadius(scale.Length() * 0.5f);

	auto mesh = std::make_unique<Mesh>();
	// Для простоты используем куб
	mesh->CreateCube(ctx);
	mesh->SetTexture(texture);
	obj->SetMesh(std::move(mesh));

	// Материал можно оставить по умолчанию или переопределить
	Material mat = obj->GetMaterial();
	mat.diffuse = Vector3(color.x, color.y, color.z);
	mat.ambient = Vector3(color.x, color.y, color.z) * 0.3f;
	obj->SetMaterial(mat);

	return obj;
}

std::pair<std::vector<std::shared_ptr<GameComponent>>, std::shared_ptr<FPSCamera>> CreateDebugScene(GameContext* ctx)
{
	std::vector<std::shared_ptr<GameComponent>> components;

	// 1. FPS камера
	auto outCamera = std::make_shared<FPSCamera>(ctx);
	outCamera->position = Vector3(0.0f, 2.0f, 8.0f);
	components.push_back(outCamera);

	// 2. Пол
	// auto floor = std::make_shared<GameComponent>(ctx);
	// auto floorMesh = std::make_unique<Mesh>();
	// floorMesh->CreatePlane(ctx);
	// // Создадим простую текстуру-шахматку (или белую)
	// auto whiteTex = Texture::CreateWhite(ctx);
	// floorMesh->SetTexture(whiteTex);
	// floor->SetMesh(std::move(floorMesh));
	// floor->scale = Vector3(20.0f, 1.0f, 20.0f);
	// floor->position = Vector3(0.0f, -0.5f, 0.0f);
	// components.push_back(floor);

	// 3. Шар
	auto sphere = std::make_shared<GameComponent>(ctx);
	sphere->position = Vector3(0.0f, 0.0f, 0.0f);
	sphere->scale = Vector3(1.2f, 1.2f, 1.2f);
	auto sphereMesh = std::make_unique<Mesh>();
	sphereMesh->CreateSphere(ctx, 32, 32);
	// Загрузим текстурку, если есть, иначе белая
	auto sphereTex = std::make_shared<Texture>();
	if (!sphereTex->LoadFromFile(ctx, L"assets/textures/minegrass.jpg")) {
		sphereTex = Texture::CreateWhite(ctx);
	}
	sphereMesh->SetTexture(sphereTex);
	sphere->SetMesh(std::move(sphereMesh));
	components.push_back(sphere);

	// 4. Шар-овал
	auto ellipsoid = std::make_shared<GameComponent>(ctx);
	ellipsoid->position = Vector3(0.0f, 1.0f, 5.0f);
	ellipsoid->scale = Vector3(0.5f, 1.7f, 0.5f);
	auto ellipsoidMesh = std::make_unique<Mesh>();
	ellipsoidMesh->CreateSphere(ctx, 32, 32);
	// Загрузим текстурку, если есть, иначе белая
	auto ellipsoidTex = std::make_shared<Texture>();
	if (!ellipsoidTex->LoadFromFile(ctx, L"assets/textures/brick.jpg")) {
		ellipsoidTex = Texture::CreateWhite(ctx);
	}
	ellipsoidMesh->SetTexture(ellipsoidTex);
	ellipsoid->SetMesh(std::move(ellipsoidMesh));
	components.push_back(ellipsoid);

	// // 4. Несколько кубов для проверки освещения и теней
	// auto cubeTex = std::make_shared<Texture>();
	// if (!cubeTex->LoadFromFile(ctx, L"assets/textures/wood.jpg")) {
	// 	cubeTex = Texture::CreateWhite(ctx);
	// }

	// // Красный куб слева
	// auto redCube = CreateTestObject(ctx, Vector3(-3.0f, 0.5f, -2.0f), Vector3(1.0f, 1.0f, 1.0f), cubeTex, Colors::Red);
	// components.push_back(redCube);

	// // Синий куб справа
	// auto blueCube = CreateTestObject(ctx, Vector3(3.0f, 0.5f, -2.0f), Vector3(1.0f, 1.0f, 1.0f), cubeTex, Colors::Blue);
	// components.push_back(blueCube);

	// // Высокий столб (для проверки теней)
	// auto pillar = std::make_shared<GameComponent>(ctx);
	// pillar->position = Vector3(0.0f, 1.5f, 4.0f);
	// pillar->scale = Vector3(0.8f, 3.0f, 0.8f);
	// auto pillarMesh = std::make_unique<Mesh>();
	// pillarMesh->CreateCube(ctx);
	// pillarMesh->SetTexture(cubeTex);
	// pillar->SetMesh(std::move(pillarMesh));
	// components.push_back(pillar);

	// // Пирамида (если есть mesh, иначе куб со скошенной текстурой – нет, просто куб)
	// auto pyramid =
	// 	CreateTestObject(ctx, Vector3(-4.0f, 0.5f, 3.0f), Vector3(1.2f, 1.2f, 1.2f), cubeTex, Colors::Yellow);
	// components.push_back(pyramid);

	return std::make_pair(components, outCamera);
}
