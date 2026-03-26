#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace Engine {

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

struct Vertex {
	Vector3 position;
	Vector4 color;
	Vector3 normal;
};

class Mesh {
   public:
	Mesh() = default;
	~Mesh() = default;

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	bool CreateCube(ID3D11Device* device)
	{
		using namespace DirectX::SimpleMath;

		// Вершины куба
		std::vector<Vertex> vertices = {
			// Front face
			{Vector3(-0.5f, -0.5f, -0.5f), Vector4(1, 0, 0, 1), Vector3(0, 0, -1)},
			{Vector3(-0.5f, 0.5f, -0.5f), Vector4(1, 0, 0, 1), Vector3(0, 0, -1)},
			{Vector3(0.5f, 0.5f, -0.5f), Vector4(1, 0, 0, 1), Vector3(0, 0, -1)},
			{Vector3(0.5f, -0.5f, -0.5f), Vector4(1, 0, 0, 1), Vector3(0, 0, -1)},

			// Back face
			{Vector3(0.5f, -0.5f, 0.5f), Vector4(0, 1, 0, 1), Vector3(0, 0, 1)},
			{Vector3(0.5f, 0.5f, 0.5f), Vector4(0, 1, 0, 1), Vector3(0, 0, 1)},
			{Vector3(-0.5f, 0.5f, 0.5f), Vector4(0, 1, 0, 1), Vector3(0, 0, 1)},
			{Vector3(-0.5f, -0.5f, 0.5f), Vector4(0, 1, 0, 1), Vector3(0, 0, 1)},

			// Top face
			{Vector3(-0.5f, 0.5f, -0.5f), Vector4(0, 0, 1, 1), Vector3(0, 1, 0)},
			{Vector3(-0.5f, 0.5f, 0.5f), Vector4(0, 0, 1, 1), Vector3(0, 1, 0)},
			{Vector3(0.5f, 0.5f, 0.5f), Vector4(0, 0, 1, 1), Vector3(0, 1, 0)},
			{Vector3(0.5f, 0.5f, -0.5f), Vector4(0, 0, 1, 1), Vector3(0, 1, 0)},

			// Bottom face
			{Vector3(-0.5f, -0.5f, 0.5f), Vector4(1, 1, 0, 1), Vector3(0, -1, 0)},
			{Vector3(-0.5f, -0.5f, -0.5f), Vector4(1, 1, 0, 1), Vector3(0, -1, 0)},
			{Vector3(0.5f, -0.5f, -0.5f), Vector4(1, 1, 0, 1), Vector3(0, -1, 0)},
			{Vector3(0.5f, -0.5f, 0.5f), Vector4(1, 1, 0, 1), Vector3(0, -1, 0)},

			// Left face
			{Vector3(-0.5f, -0.5f, 0.5f), Vector4(1, 0, 1, 1), Vector3(-1, 0, 0)},
			{Vector3(-0.5f, 0.5f, 0.5f), Vector4(1, 0, 1, 1), Vector3(-1, 0, 0)},
			{Vector3(-0.5f, 0.5f, -0.5f), Vector4(1, 0, 1, 1), Vector3(-1, 0, 0)},
			{Vector3(-0.5f, -0.5f, -0.5f), Vector4(1, 0, 1, 1), Vector3(-1, 0, 0)},

			// Right face
			{Vector3(0.5f, -0.5f, -0.5f), Vector4(0, 1, 1, 1), Vector3(1, 0, 0)},
			{Vector3(0.5f, 0.5f, -0.5f), Vector4(0, 1, 1, 1), Vector3(1, 0, 0)},
			{Vector3(0.5f, 0.5f, 0.5f), Vector4(0, 1, 1, 1), Vector3(1, 0, 0)},
			{Vector3(0.5f, -0.5f, 0.5f), Vector4(0, 1, 1, 1), Vector3(1, 0, 0)},
		};

		// Индексы
		std::vector<UINT> indices = {
			0,	1,	2,	0,	2,	3,	 // Front
			4,	5,	6,	4,	6,	7,	 // Back
			8,	9,	10, 8,	10, 11,	 // Top
			12, 13, 14, 12, 14, 15,	 // Bottom
			16, 17, 18, 16, 18, 19,	 // Left
			20, 21, 22, 20, 22, 23	 // Right
		};

		m_indexCount = static_cast<UINT>(indices.size());

		// Создание vertex buffer
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_DEFAULT;
		vbDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertices.size());
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vbData = {};
		vbData.pSysMem = vertices.data();

		HRESULT hr = device->CreateBuffer(&vbDesc, &vbData, m_vertexBuffer.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// Создание index buffer
		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_DEFAULT;
		ibDesc.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA ibData = {};
		ibData.pSysMem = indices.data();

		hr = device->CreateBuffer(&ibDesc, &ibData, m_indexBuffer.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		return true;
	}

	void Bind(ID3D11DeviceContext* context) const
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Draw(ID3D11DeviceContext* context) const { context->DrawIndexed(m_indexCount, 0, 0); }

   private:
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	UINT m_indexCount = 0;
};

}  // namespace Engine
