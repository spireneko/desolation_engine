#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "Colors.hpp"
#include "GameContext.hpp"

using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

struct Vertex {
	Vector3 position;
	Vector4 color;
};

class Mesh {
   public:
	Mesh();
	~Mesh();

	bool CreateCube(GameContext* context);
	void Draw(GameContext* context);

	size_t GetIndexCount() const { return indices.size(); }

   private:
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<WORD> indices;
};
