#include "ShaderManager.hpp"

#include <SimpleMath.h>
#include <d3dcompiler.h>
#include <stdexcept>

using namespace DirectX::SimpleMath;

const char* vertexShaderSrc = R"(
cbuffer ConstantBuffer : register(b0) {
    matrix world;
    matrix view;
    matrix projection;
};

struct VS_INPUT {
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(input.pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    output.color = input.color;
    return output;
}
)";

const char* pixelShaderSrc = R"(
struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

float4 main(PS_INPUT input) : SV_Target {
    return input.color;
}
)";

bool ShaderManager::Initialize(ID3D11Device* device)
{
	ComPtr<ID3DBlob> vsBlobPtr, psBlobPtr, errorBlob;

	// Компиляция вершинного шейдера
	HRESULT hr = D3DCompile(
		vertexShaderSrc,
		strlen(vertexShaderSrc),
		nullptr,
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		0,
		0,
		&vsBlobPtr,
		&errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		return false;
	}

	// Компиляция пиксельного шейдера
	hr = D3DCompile(
		pixelShaderSrc,
		strlen(pixelShaderSrc),
		nullptr,
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		0,
		0,
		&psBlobPtr,
		&errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		return false;
	}

	// Создание шейдеров
	device->CreateVertexShader(vsBlobPtr->GetBufferPointer(), vsBlobPtr->GetBufferSize(), nullptr, &vertexShader);
	device->CreatePixelShader(psBlobPtr->GetBufferPointer(), psBlobPtr->GetBufferSize(), nullptr, &pixelShader);

	// Input Layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	device->CreateInputLayout(layout, 2, vsBlobPtr->GetBufferPointer(), vsBlobPtr->GetBufferSize(), &inputLayout);

	// Constant Buffer
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(Matrix) * 3;	 // world, view, projection
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	device->CreateBuffer(&cbd, nullptr, &constantBuffer);

	return true;
}

void ShaderManager::Apply(ID3D11DeviceContext* context)
{
	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	context->PSSetShader(pixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(inputLayout.Get());
	context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
}

void ShaderManager::UpdateConstants(ID3D11DeviceContext* context, const void* data, size_t size)
{
	context->UpdateSubresource(constantBuffer.Get(), 0, nullptr, data, 0, 0);
}
