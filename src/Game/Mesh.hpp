#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <string>
#include <vector>

#include "Colors.hpp"
#include "GameContext.hpp"
#include "Texture.hpp"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
	Vector4 color;
};

class Mesh {
   public:
	Mesh();
	~Mesh();

	bool CreateCube(GameContext* context);
	void CreatePlane(GameContext* context);
	bool CreateSphere(GameContext* context, int slices = 16, int stacks = 16);
	bool CreateGrid(
		GameContext* context, float size = 100.0f, int divisions = 20, const Vector4& color = Colors::LightGray
	);
	bool LoadFromObj(
		GameContext* context, const std::string& filePath, const std::string& baseDir = "",
		std::shared_ptr<Texture> texture = nullptr
	);
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
	void SetTexture(std::shared_ptr<Texture> texture);
	void Draw(GameContext* context);

	size_t GetIndexCount() const { return indices.size(); }

   private:
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	std::shared_ptr<Texture> texture;
};
