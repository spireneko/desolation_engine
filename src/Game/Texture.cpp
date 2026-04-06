#include "Texture.hpp"

#include <DirectXTex.h>
#include <iostream>

bool Texture::LoadFromFile(GameContext* context, const std::wstring& filePath)
{
	auto device = context->GetGraphicsDevice();
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;

	HRESULT hr = DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
	if (FAILED(hr)) {
		std::wcerr << L"Failed to load texture: " << filePath << std::endl;
		return false;
	}

	hr = DirectX::CreateShaderResourceView(
		device, image.GetImages(), image.GetImageCount(), metadata, textureView.GetAddressOf()
	);
	if (FAILED(hr)) {
		std::wcerr << L"Failed to create texture SRV: " << filePath << std::endl;
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* Texture::GetView() const
{
	return textureView.Get();
}

std::shared_ptr<Texture> Texture::CreateWhite(GameContext* context)
{
	auto white = std::make_shared<Texture>();
	white->textureView = CreateWhiteView(context->GetGraphicsDevice());
	return white;
}

ComPtr<ID3D11ShaderResourceView> Texture::CreateWhiteView(ID3D11Device* device)
{
	static ComPtr<ID3D11ShaderResourceView> whiteView;
	if (whiteView) {
		return whiteView;
	}

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	UINT color[1] = {0xFFFFFFFF};
	initData.pSysMem = color;
	initData.SysMemPitch = sizeof(color);

	ComPtr<ID3D11Texture2D> texture;
	if (FAILED(device->CreateTexture2D(&desc, &initData, &texture))) {
		return nullptr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(texture.Get(), &srvDesc, &whiteView);
	return whiteView;
}
