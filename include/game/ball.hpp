#pragma once

#include "moveable_object.hpp"
#include "players.hpp"

const float START_SPEED = 0.7;

class Ball : public MoveableObject {
   public:
	Ball(Gameable* game, Vector3 position, Vector3 velocity) : MoveableObject(game)
	{
		m_scale = Vector3(0.02, 0.02, 1.0);
		m_position = position;

		m_velocity = velocity;
		m_velocity.Normalize();

		m_speed = START_SPEED;

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
};

void Ball::CreateVertexBuffer(ID3D11Device* device)
{
	Vertex vertices[] = {
		{Vector3(-1.0, -1.0, 0.0), WHITE},
		{Vector3(1.0, -1.0, 0.0), WHITE},
		{Vector3(-1.0, 1.0, 0.0), WHITE},
		{Vector3(1.0, 1.0, 0.0), WHITE}
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

void Ball::CreateIndexBuffer(ID3D11Device* device)
{
	int indices[] = {0, 2, 1, 1, 2, 3};
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

void Ball::UpdatePosition(float deltaTime)
{
	bool scored = false;

	m_position += m_velocity * m_speed * deltaTime;

	const float half = m_scale.x;

	// Отскок от стен (верх/низ)
	if (m_position.y + half > 1.0f) {
		m_velocity.y = -m_velocity.y;
		m_position.y = 1.0f - half;
	}
	if (m_position.y - half < -1.0f) {
		m_velocity.y = -m_velocity.y;
		m_position.y = -1.0f + half;
	}

	// По бокам увеличиваем счёт
	if (m_position.x + half > 1.0f) {
		scored = true;
		m_game->AddScorePlayer1();
	}
	if (m_position.x - half < -1.0f) {
		scored = true;
		m_game->AddScorePlayer2();
	}

	if (scored) {
		m_game->Restart();
		return;
	}

	for (const auto& comp : m_game->GetComponents()) {
		auto* pad = dynamic_cast<PlayerPad*>(comp.get());
		if (!pad) {
			continue;
		}

		Vector3 bPos = m_position;
		Vector3 pPos = pad->GetPosition();
		Vector3 bScl = m_scale;
		Vector3 pScl = pad->GetScale();

		if (GetBoundingBox().Intersects(pad->GetBoundingBox())) {
			m_velocity.x = -m_velocity.x;

			// Угол удара (чем выше/ниже по пэду — тем сильнее меняется vy)
			float hitFactor = (bPos.y - pPos.y) / (pScl.y * 0.5f);
			m_velocity.y += hitFactor * 1.2f;
			m_velocity.Normalize();

			m_speed += 0.08;
		}
	}
}

void Ball::Restart()
{
	m_position.x = 0;
	m_position.y = 0;

	m_speed = START_SPEED;

	m_velocity = Vector3(-1.0, -0.5, 0.0);
	m_velocity.Normalize();
}

DirectX::BoundingBox Ball::GetBoundingBox() const
{
	DirectX::XMFLOAT3 center = {m_position.x, m_position.y, m_position.z};
	DirectX::XMFLOAT3 extents = {m_scale.x, m_scale.y, 0.01f};

	return DirectX::BoundingBox(center, extents);
}
