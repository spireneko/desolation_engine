#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <iostream>
#include <stdexcept>

#include "vertex.hpp"

using Microsoft::WRL::ComPtr;

class ShaderManager {
   public:
	explicit ShaderManager(ID3D11Device* device);
	~ShaderManager() = default;

	ID3D11VertexShader* GetVertexShader() const { return m_vertexShader.Get(); }

	ID3D11PixelShader* GetPixelShader() const { return m_pixelShader.Get(); }

	ID3D11InputLayout* GetInputLayout() const { return m_inputLayout.Get(); }

   private:
	void CompileAndCreateShaders(ID3D11Device* device);

	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
};

ShaderManager::ShaderManager(ID3D11Device* device)
{
	if (!device) {
		throw std::runtime_error("ShaderManager: device is null");
	}

	CompileAndCreateShaders(device);
}

void ShaderManager::CompileAndCreateShaders(ID3D11Device* device)
{
	HRESULT hr;

	// === Vertex Shader ===
	ComPtr<ID3DBlob> vsBlob, errorBlob;
	hr = D3DCompileFromFile(
		L"./shaders/MyVeryFirstShader.hlsl",
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vsBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
		}
		throw std::runtime_error("Failed to compile vertex shader");
	}

	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create vertex shader");
	}

	// === Pixel Shader ===
	ComPtr<ID3DBlob> psBlob;
	hr = D3DCompileFromFile(
		L"./shaders/MyVeryFirstShader.hlsl",
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob,
		&errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			std::cerr << (char*)errorBlob->GetBufferPointer() << std::endl;
		}
		throw std::runtime_error("Failed to compile pixel shader");
	}

	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create pixel shader");
	}

	// === Input Layout ===
	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = device->CreateInputLayout(
		inputElements, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout
	);

	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create input layout");
	}
}
