#pragma once

#include "moveable_object.hpp"

class Divider : public MoveableObject {
   public:
	Divider(Vector3 position)
	{
		m_scale = Vector3(0.01, 0.03, 1.0);
		m_velocity = Vector3::Zero;
		m_speed = 0.0;
		m_position = position;
	}

	virtual void Restart() {};
	virtual void UpdatePosition(float deltaTime) {};

   protected:
	void CreateVertexBuffer(ID3D11Device* device) override;
	void CreateIndexBuffer(ID3D11Device* device) override;
};

void Divider::CreateVertexBuffer(ID3D11Device* device)
{
	Vertex vertices[] = {
		{Vector3(-1.0, 1.0, 0.0f), GREY},
		{Vector3(1.0, 1.0, 0.0f), GREY},
		{Vector3(-1.0, -1.0, 0.0f), GREY},
		{Vector3(1.0, -1.0, 0.0f), GREY}
	};
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.ByteWidth = sizeof(vertices);
	D3D11_SUBRESOURCE_DATA vdata = {vertices, 0, 0};
	HRESULT hr = device->CreateBuffer(&vbd, &vdata, &m_vertexBuffer);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create vertex buffer");
	}
}

void Divider::CreateIndexBuffer(ID3D11Device* device)
{
	int indices[] = {0, 1, 2, 1, 3, 2};
	m_indexCount = 6;
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.ByteWidth = sizeof(indices);
	D3D11_SUBRESOURCE_DATA idata = {indices, 0, 0};
	HRESULT hr = device->CreateBuffer(&ibd, &idata, &m_indexBuffer);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create index buffer");
	}
}
