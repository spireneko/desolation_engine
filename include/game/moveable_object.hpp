#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <iostream>

#include "colors.hpp"
#include "game_component.hpp"
#include "vertex.hpp"

using namespace DirectX::SimpleMath;
using namespace Microsoft::WRL;

class MoveableObject : public GameComponent {
   public:
	explicit MoveableObject(Gameable* game) : GameComponent(game), m_orientation(Quaternion::Identity)
	{
		ID3D11Device* device = m_game->GetDevice();

		CreateConstantBuffer(device);
		CreateRasterizerState(device);
		UpdateWorldMatrix();
	}

	virtual ~MoveableObject() = default;

	virtual void Update(float deltaTime) override;
	virtual void Draw() override;
	virtual void Shutdown() override;

	Vector3 GetPosition() const { return m_position; }

	Vector3 GetScale() const { return m_scale; }

	Vector3 GetVelocity() const { return m_velocity; }

	void SetPosition(const Vector3& pos) { m_position = pos; }

	void SetVelocity(const Vector3& vel) { m_velocity = vel; }

	Vector3 GetRotation() const;
	void SetRotation(const Vector3& anglesDegrees);

   protected:
	virtual void CreateVertexBuffer(ID3D11Device* device) = 0;
	virtual void CreateIndexBuffer(ID3D11Device* device) = 0;
	virtual void UpdatePosition(float deltaTime) = 0;

	virtual void UpdateWorldMatrix();

	// Рендеринг
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

	// Трансформация
	Vector3 m_position{0.0f, 0.0f, 0.0f};
	Vector3 m_scale{1.0f, 1.0f, 1.0f};
	Vector3 m_velocity{0.0f, 0.0f, 0.0f};
	float m_speed;

	UINT m_indexCount = 0;

   private:
	void CreateConstantBuffer(ID3D11Device* device);
	void UpdateConstantBuffer();
	void CreateRasterizerState(ID3D11Device* device);

	// Матрица поворота
	Quaternion m_orientation;

	Matrix m_worldMatrix;
};

void MoveableObject::Update(float deltaTime)
{
	UpdatePosition(deltaTime);
	UpdateWorldMatrix();
	UpdateConstantBuffer();
}

void MoveableObject::Draw()
{
	auto context = m_game->GetContext();

	context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	context->IASetInputLayout(m_game->GetInputLayout());
	context->VSSetShader(m_game->GetVertexShader(), nullptr, 0);
	context->PSSetShader(m_game->GetPixelShader(), nullptr, 0);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(m_rasterizerState.Get());

	context->DrawIndexed(m_indexCount, 0, 0);
}

Vector3 MoveableObject::GetRotation() const
{
	// Из кватерниона получаем углы Эйлера (в радианах)
	Vector3 euler = m_orientation.ToEuler();

	// Возвращаем в градусах
	return Vector3(
		DirectX::XMConvertToDegrees(euler.x), DirectX::XMConvertToDegrees(euler.y), DirectX::XMConvertToDegrees(euler.z)
	);
}

void MoveableObject::SetRotation(const Vector3& anglesDegrees)
{
	// Преобразуем градусы в радианы
	float yaw = DirectX::XMConvertToRadians(anglesDegrees.x);
	float pitch = DirectX::XMConvertToRadians(anglesDegrees.y);
	float roll = DirectX::XMConvertToRadians(anglesDegrees.z);
	m_orientation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
}

void MoveableObject::UpdateWorldMatrix()
{
	m_worldMatrix = Matrix::CreateScale(m_scale) * Matrix::CreateFromQuaternion(m_orientation) *
					Matrix::CreateTranslation(m_position);
}

void MoveableObject::Shutdown() {}

void MoveableObject::CreateConstantBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = sizeof(Matrix);
	HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create constant buffer");
	}
}

void MoveableObject::UpdateConstantBuffer()
{
	m_game->GetContext()->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &m_worldMatrix, 0, 0);
}

void MoveableObject::CreateRasterizerState(ID3D11Device* device)
{
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	HRESULT hr = device->CreateRasterizerState(&rastDesc, &m_rasterizerState);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create rasterizer state");
	}
}
