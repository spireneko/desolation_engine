#pragma once

#include "moveable_object.hpp"

enum Players { PLAYER1, PLAYER2 };

class PlayerPad : public ControlledObject {
   public:
	PlayerPad(Players player) : m_player(player), m_speed(2.0)	// скорость в единицах/сек
	{
		m_scale = Vector3(0.08, 0.30, 1.0);	 // ширина × высота

		if (player == PLAYER1) {
			m_position.x = -0.92f;
		} else {
			m_position.x = +0.92f;
		}

		// Начальная позиция по Y = 0
	}

   protected:
	void CreateVertexBuffer(ID3D11Device* device) override;
	void CreateIndexBuffer(ID3D11Device* device) override;
	void UpdatePosition(float deltaTime) override;

   private:
	Players m_player;
	float m_speed;
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
	const float halfHeight = m_scale.y * 0.5f;
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
		{Vector3(-0.05f, 0.15f, 0.0f), WHITE},
		{Vector3(0.05f, 0.15f, 0.0f), WHITE},
		{Vector3(-0.05f, -0.15f, 0.0f), WHITE},
		{Vector3(0.05f, -0.15f, 0.0f), WHITE}
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
