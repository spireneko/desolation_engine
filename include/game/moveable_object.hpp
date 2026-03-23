#pragma once

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
	virtual ~MoveableObject() = default;

	virtual void Init(Gameable* game) override;
	virtual void Update(float deltaTime) override;
	virtual void Draw() override;
	virtual void Shutdown() override;

	Vector3 GetPosition() const { return m_position; }

	Vector3 GetScale() const { return m_scale; }

	Vector3 GetVelocity() const { return m_velocity; }

	void SetPosition(const Vector3& pos) { m_position = pos; }

	void SetVelocity(const Vector3& vel) { m_velocity = vel; }

   protected:
	virtual void CreateVertexBuffer(ID3D11Device* device) = 0;
	virtual void CreateIndexBuffer(ID3D11Device* device) = 0;
	virtual void UpdatePosition(float deltaTime) = 0;

	virtual void UpdateWorldMatrix();

	Gameable* m_game = nullptr;

	// Рендеринг
	ComPtr<ID3D11Buffer> m_vertexBuffer;
	ComPtr<ID3D11Buffer> m_indexBuffer;
	ComPtr<ID3D11Buffer> m_constantBuffer;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;

	// Трансформация
	Matrix m_worldMatrix;
	Vector3 m_position{0.0f, 0.0f, 0.0f};
	Vector3 m_scale{1.0f, 1.0f, 1.0f};
	Vector3 m_velocity{0.0f, 0.0f, 0.0f};
	float m_speed;

	UINT m_indexCount = 0;

   private:
	void CreateConstantBuffer(ID3D11Device* device);
	void UpdateConstantBuffer();
	void CreateShadersAndLayout(ID3D11Device* device);
	void CreateRasterizerState(ID3D11Device* device);
};

void MoveableObject::Init(Gameable* game)
{
	m_game = game;
	ID3D11Device* device = game->GetDevice();

	CreateVertexBuffer(device);
	CreateIndexBuffer(device);
	CreateConstantBuffer(device);
	CreateShadersAndLayout(device);
	CreateRasterizerState(device);

	// Начальная матрица будет сформирована в первом Update
	UpdateWorldMatrix();
}

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

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	context->RSSetState(m_rasterizerState.Get());

	context->DrawIndexed(m_indexCount, 0, 0);
}

void MoveableObject::UpdateWorldMatrix()
{
	m_worldMatrix = Matrix::CreateScale(m_scale) * Matrix::CreateTranslation(m_position);
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

void MoveableObject::CreateShadersAndLayout(ID3D11Device* device)
{
	ComPtr<ID3DBlob> vertexBC, errorVertexCode;
	HRESULT hr = D3DCompileFromFile(
		L"./shaders/MyVeryFirstShader.hlsl",
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode
	);
	if (FAILED(hr)) {
		if (errorVertexCode) {
			std::cerr << (char*)errorVertexCode->GetBufferPointer() << std::endl;
		}
		throw std::runtime_error("Failed to compile vertex shader");
	}
	hr = device->CreateVertexShader(vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create vertex shader");
	}

	ComPtr<ID3DBlob> pixelBC, errorPixelCode;
	hr = D3DCompileFromFile(
		L"./shaders/MyVeryFirstShader.hlsl",
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelBC,
		&errorPixelCode
	);
	if (FAILED(hr)) {
		if (errorPixelCode) {
			std::cerr << (char*)errorPixelCode->GetBufferPointer() << std::endl;
		}
		throw std::runtime_error("Failed to compile pixel shader");
	}
	hr = device->CreatePixelShader(pixelBC->GetBufferPointer(), pixelBC->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create pixel shader");
	}

	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = device->CreateInputLayout(
		inputElements, 2, vertexBC->GetBufferPointer(), vertexBC->GetBufferSize(), &m_inputLayout
	);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create input layout");
	}
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
