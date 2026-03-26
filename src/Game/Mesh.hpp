#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

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

	bool CreateCube(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* context);

	size_t GetIndexCount() const { return indices.size(); }

   private:
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<WORD> indices;
};
