#include "ShaderManager.hpp"

ShaderManager::ShaderManager(GameContext* ctx) : gameContext(ctx)
{
	ComPtr<ID3DBlob> vsBlobPtr, psBlobPtr, errorBlob;
	ID3D11Device* device = gameContext->GetGraphicsDevice();

	// Компиляция вершинного шейдера
	HRESULT hr = D3DCompileFromFile(
		L"shaders/VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBlobPtr, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", (char*)errorBlob->GetBufferPointer());
		}
		throw std::runtime_error("Failed to compile vertex shader");
	}

	// Компиляция пиксельного шейдера
	hr = D3DCompileFromFile(
		L"shaders/PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBlobPtr, &errorBlob
	);
	if (FAILED(hr)) {
		if (errorBlob) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", (char*)errorBlob->GetBufferPointer());
		}
		throw std::runtime_error("Failed to compile pixel shader");
	}

	// Создание шейдеров
	device->CreateVertexShader(vsBlobPtr->GetBufferPointer(), vsBlobPtr->GetBufferSize(), nullptr, &vertexShader);
	device->CreatePixelShader(psBlobPtr->GetBufferPointer(), psBlobPtr->GetBufferSize(), nullptr, &pixelShader);

	// Input Layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	device->CreateInputLayout(layout, 4, vsBlobPtr->GetBufferPointer(), vsBlobPtr->GetBufferSize(), &inputLayout);

	// Constant Buffer
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(PerFrameConstants);
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	device->CreateBuffer(&cbd, nullptr, &constantBuffer);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &samplerState);
}

void ShaderManager::Apply()
{
	auto context = gameContext->GetGraphicsContext();

	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	context->PSSetShader(pixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(inputLayout.Get());
	context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
	context->PSSetSamplers(0, 1, samplerState.GetAddressOf());
}

void ShaderManager::UpdateConstants(const void* data)
{
	auto device = gameContext->GetGraphicsContext();
	device->UpdateSubresource(constantBuffer.Get(), 0, nullptr, data, 0, 0);
}
