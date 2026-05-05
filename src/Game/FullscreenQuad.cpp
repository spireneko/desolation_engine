#include "FullscreenQuad.hpp"

struct ScreenVertex {
	float x, y, z, w;
	float u, v;
};

bool FullscreenQuad::Initialize(ID3D11Device* device)
{
	ScreenVertex vertices[4] = {
		{-1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{-1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
		{1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	return SUCCEEDED(device->CreateBuffer(&bd, &initData, &vertexBuffer));
}

void FullscreenQuad::Draw(ID3D11DeviceContext* ctx)
{
	UINT stride = sizeof(ScreenVertex);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	ctx->Draw(4, 0);
}
