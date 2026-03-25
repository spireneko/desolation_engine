#pragma once

#include "input_device.hpp"
#include "moveable_object.hpp"

enum Players { PLAYER1, PLAYER2 };

class PlayerPad : public MoveableObject {
   public:
	PlayerPad(Gameable* game, Players player) : MoveableObject(game), m_player(player)
	{
		m_scale = Vector3(0.015, 0.07, 1.0);
		m_speed = 1.0;

		if (player == PLAYER1) {
			m_position.x = -0.92f;
		} else {
			m_position.x = +0.92f;
		}

		ID3D11Device* device = m_game->GetDevice();

		CreateVertexBuffer(device);
		CreateIndexBuffer(device);
	}

	DirectX::BoundingBox GetBoundingBox() const;

   protected:
	void CreateVertexBuffer(ID3D11Device* device) override;
	void CreateIndexBuffer(ID3D11Device* device) override;
	void UpdatePosition(float deltaTime) override;
	void Restart() override;

   private:
	Players m_player;
};

void PlayerPad::UpdatePosition(float deltaTime)
{
	float dy = 0.0f;

	auto& input = InputDevice::Get();

	if (m_player == PLAYER1) {
		if (input.IsKeyDown('W')) {
			dy = +m_speed * deltaTime;
		}
		if (input.IsKeyDown('S')) {
			dy = -m_speed * deltaTime;
		}
	} else {
		if (input.IsKeyDown(VK_UP)) {
			dy = +m_speed * deltaTime;
		}
		if (input.IsKeyDown(VK_DOWN)) {
			dy = -m_speed * deltaTime;
		}
	}

	m_position.y += dy;

	// Ограничение по вертикали
	const float halfHeight = m_scale.y;
	const float maxY = 1.0f - halfHeight;
	const float minY = -maxY;

	if (m_position.y > maxY) {
		m_position.y = maxY;
	}
	if (m_position.y < minY) {
		m_position.y = minY;
	}
}

void PlayerPad::CreateVertexBuffer(ID3D11Device* device)
{
	Vertex vertices[] = {
		{Vector3(-1.0, 1.0, 0.0f), WHITE},
		{Vector3(1.0, 1.0, 0.0f), WHITE},
		{Vector3(-1.0, -1.0, 0.0f), WHITE},
		{Vector3(1.0, -1.0, 0.0f), WHITE}
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

void PlayerPad::CreateIndexBuffer(ID3D11Device* device)
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

void PlayerPad::Restart()
{
	m_position.y = 0;
}

DirectX::BoundingBox PlayerPad::GetBoundingBox() const
{
	DirectX::XMFLOAT3 center = {m_position.x, m_position.y, m_position.z};
	DirectX::XMFLOAT3 extents = {m_scale.x, m_scale.y, 0.01f};

	return DirectX::BoundingBox(center, extents);
}
