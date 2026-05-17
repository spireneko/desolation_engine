#include "ShaderManager.hpp"

ShaderManager::ShaderManager(GameContext* ctx) : gameContext(ctx)
{
	ID3D11Device* device = gameContext->GetGraphicsDevice();

	// Input layout shared by GBuffer and Forward
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// GBuffer pass
	CreatePass(
		PassType::GBuffer,
		L"shaders/GBufferVertexShader.hlsl",
		L"shaders/GBufferPixelShader.hlsl",
		sizeof(PerFrameConstants),
		layout,
		4
	);

	// Forward pass (legacy shaders)
	CreatePass(
		PassType::Forward,
		L"shaders/VertexShader.hlsl",
		L"shaders/PixelShader.hlsl",
		sizeof(PerFrameConstants),
		layout,
		4
	);

	// Deferred directional (fullscreen quad - no input layout needed)
	CreatePass(
		PassType::DeferredDirectional,
		L"shaders/FullscreenQuadVS.hlsl",
		L"shaders/DeferredDirectional.hlsl",
		sizeof(LightingConstants)
	);

	// Deferred point volume
	D3D11_INPUT_ELEMENT_DESC volumeLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	CreatePass(
		PassType::DeferredPoint,
		L"shaders/PointVolumeSphereVS.hlsl",
		L"shaders/DeferredPointVolume.hlsl",
		sizeof(PointLightConstants),
		volumeLayout,
		1
	);

	// Deferred spot volume
	D3D11_INPUT_ELEMENT_DESC spotLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	CreatePass(
		PassType::DeferredSpot,
		L"shaders/SpotVolumeVS.hlsl",
		L"shaders/DeferredSpotVolume.hlsl",
		sizeof(SpotLightConstants),
		spotLayout,
		1
	);

	// Samplers
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &samplerState);

	D3D11_SAMPLER_DESC pointDesc = {};
	pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&pointDesc, &pointSamplerState);

	D3D11_SAMPLER_DESC shadowDesc = {};
	shadowDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowDesc.BorderColor[0] = 1.0f;
	shadowDesc.BorderColor[1] = 1.0f;
	shadowDesc.BorderColor[2] = 1.0f;
	shadowDesc.BorderColor[3] = 1.0f;
	shadowDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	device->CreateSamplerState(&shadowDesc, &shadowSamplerState);

	// Shadow shaders (legacy)
	ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
	HRESULT hr = D3DCompileFromFile(
		L"shaders/ShadowVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		0,
		0,
		&vsBlob,
		&errorBlob
	);
	if (SUCCEEDED(hr)) {
		device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &shadowVertexShader);
	}

	hr = D3DCompileFromFile(
		L"shaders/ShadowPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		0,
		0,
		&psBlob,
		&errorBlob
	);
	if (SUCCEEDED(hr)) {
		device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &shadowPixelShader);
	}

	D3D11_BUFFER_DESC objCbd = {};
	objCbd.ByteWidth = sizeof(PerObjectConstants);
	objCbd.Usage = D3D11_USAGE_DEFAULT;
	objCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	device->CreateBuffer(&objCbd, nullptr, &perObjectConstantBuffer);

	D3D11_BUFFER_DESC shadowCbd = {};
	shadowCbd.ByteWidth = sizeof(Matrix) * 2;
	shadowCbd.Usage = D3D11_USAGE_DEFAULT;
	shadowCbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	device->CreateBuffer(&shadowCbd, nullptr, &shadowConstantBuffer);
}

bool ShaderManager::CompileShader(const wchar_t* path, const char* entry, const char* target, ComPtr<ID3DBlob>& blob)
{
	ComPtr<ID3DBlob> errorBlob;
	HRESULT hr =
		D3DCompileFromFile(path, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, 0, 0, &blob, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", (char*)errorBlob->GetBufferPointer());
		}
		return false;
	}
	return true;
}

bool ShaderManager::CreatePass(
	PassType type, const wchar_t* vsPath, const wchar_t* psPath, size_t constantSize,
	const D3D11_INPUT_ELEMENT_DESC* layout, UINT layoutCount
)
{
	ID3D11Device* device = gameContext->GetGraphicsDevice();
	PassData pass;
	pass.constantSize = constantSize;

	ComPtr<ID3DBlob> vsBlob, psBlob;
	if (!CompileShader(vsPath, "main", "vs_5_0", vsBlob)) {
		return false;
	}
	if (!CompileShader(psPath, "main", "ps_5_0", psBlob)) {
		return false;
	}

	device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &pass.vs);
	device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pass.ps);

	if (layout && layoutCount > 0) {
		device->CreateInputLayout(
			layout, layoutCount, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &pass.layout
		);
	}

	if (constantSize > 0) {
		D3D11_BUFFER_DESC cbd = {};
		cbd.ByteWidth = static_cast<UINT>(constantSize);
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		device->CreateBuffer(&cbd, nullptr, &pass.constantBuffer);
	}

	passes[type] = std::move(pass);
	return true;
}

void ShaderManager::Apply(PassType passType)
{
	auto it = passes.find(passType);
	if (it == passes.end()) {
		return;
	}

	auto& pass = it->second;
	auto ctx = gameContext->GetGraphicsContext();

	ctx->VSSetShader(pass.vs.Get(), nullptr, 0);
	ctx->PSSetShader(pass.ps.Get(), nullptr, 0);

	if (pass.layout) {
		ctx->IASetInputLayout(pass.layout.Get());
	}

	if (pass.constantBuffer) {
		ctx->VSSetConstantBuffers(0, 1, pass.constantBuffer.GetAddressOf());
		ctx->PSSetConstantBuffers(0, 1, pass.constantBuffer.GetAddressOf());
	}

	if (passType == PassType::GBuffer || passType == PassType::Forward) {
		ctx->VSSetConstantBuffers(1, 1, perObjectConstantBuffer.GetAddressOf());
		ctx->PSSetConstantBuffers(1, 1, perObjectConstantBuffer.GetAddressOf());
	}

	// Bind appropriate samplers
	if (passType == PassType::DeferredDirectional || passType == PassType::DeferredPoint ||
		passType == PassType::DeferredSpot) {
		ctx->PSSetSamplers(0, 1, pointSamplerState.GetAddressOf());
		ctx->PSSetSamplers(1, 1, shadowSamplerState.GetAddressOf());
	} else {
		ctx->PSSetSamplers(0, 1, samplerState.GetAddressOf());
	}
}

void ShaderManager::UpdateConstants(PassType passType, const void* data, size_t size)
{
	auto it = passes.find(passType);
	if (it == passes.end() || !it->second.constantBuffer) {
		return;
	}

	auto ctx = gameContext->GetGraphicsContext();
	ctx->UpdateSubresource(it->second.constantBuffer.Get(), 0, nullptr, data, 0, 0);
}

void ShaderManager::UpdatePerObjectConstants(const void* data, size_t size)
{
	auto ctx = gameContext->GetGraphicsContext();
	ctx->UpdateSubresource(perObjectConstantBuffer.Get(), 0, nullptr, data, 0, 0);
}

void ShaderManager::ApplyShadow()
{
	auto ctx = gameContext->GetGraphicsContext();
	ctx->VSSetShader(shadowVertexShader.Get(), nullptr, 0);
	ctx->PSSetShader(shadowPixelShader.Get(), nullptr, 0);
	// Use GBuffer layout or create compatible
	auto it = passes.find(PassType::GBuffer);
	if (it != passes.end() && it->second.layout) {
		ctx->IASetInputLayout(it->second.layout.Get());
	}
	ctx->VSSetConstantBuffers(0, 1, shadowConstantBuffer.GetAddressOf());
}

void ShaderManager::UpdateShadowConstants(const Matrix& world, const Matrix& viewProj)
{
	struct ShadowCB {
		Matrix world;
		Matrix viewProj;
	};

	ShadowCB cb;
	cb.world = world.Transpose();
	cb.viewProj = viewProj.Transpose();
	gameContext->GetGraphicsContext()->UpdateSubresource(shadowConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
}
