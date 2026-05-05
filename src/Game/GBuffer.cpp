#include "GBuffer.hpp"

#include <stdexcept>

bool GBuffer::Initialize(ID3D11Device* device, int w, int h)
{
	Shutdown();
	width = w;
	height = h;

	if (!CreateAttachment(device, Attachment::AlbedoMetallic, w, h)) {
		return false;
	}
	if (!CreateAttachment(device, Attachment::NormalRoughness, w, h)) {
		return false;
	}
	if (!CreateAttachment(device, Attachment::EmissiveAO, w, h)) {
		return false;
	}
	if (!CreateDepthBuffer(device, w, h)) {
		return false;
	}

	initialized = true;
	return true;
}

void GBuffer::Shutdown()
{
	for (auto& target : colorTargets) {
		target.texture.Reset();
		target.rtv.Reset();
		target.srv.Reset();
	}
	depthTexture.Reset();
	depthDSV.Reset();
	depthReadOnlyDSV.Reset();
	depthSRV.Reset();
	initialized = false;
}

void GBuffer::Resize(int w, int h)
{
	if (!initialized || width == w && height == h) {
		return;
	}

	ComPtr<ID3D11Device> device;
	colorTargets[0].texture->GetDevice(&device);
	Initialize(device.Get(), w, h);
}

bool GBuffer::CreateAttachment(ID3D11Device* device, Attachment attachment, int w, int h)
{
	size_t idx = static_cast<size_t>(attachment);

	DXGI_FORMAT format;
	switch (attachment) {
		case Attachment::AlbedoMetallic:
			format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			break;
		case Attachment::NormalRoughness:
			format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case Attachment::EmissiveAO:
			format = DXGI_FORMAT_R11G11B10_FLOAT;
			break;
		default:
			return false;
	}

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = w;
	texDesc.Height = h;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &colorTargets[idx].texture);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(colorTargets[idx].texture.Get(), &rtvDesc, &colorTargets[idx].rtv);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(colorTargets[idx].texture.Get(), &srvDesc, &colorTargets[idx].srv);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool GBuffer::CreateDepthBuffer(ID3D11Device* device, int w, int h)
{
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = w;
	depthDesc.Height = h;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = device->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, &depthDSV);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC readOnlyDesc = {};
	readOnlyDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	readOnlyDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	readOnlyDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
	hr = device->CreateDepthStencilView(depthTexture.Get(), &readOnlyDesc, &depthReadOnlyDSV);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(depthTexture.Get(), &srvDesc, &depthSRV);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void GBuffer::BindForGeometryPass(ID3D11DeviceContext* ctx)
{
	ID3D11RenderTargetView* rtvs[3] = {colorTargets[0].rtv.Get(), colorTargets[1].rtv.Get(), colorTargets[2].rtv.Get()};
	ctx->OMSetRenderTargets(3, rtvs, depthDSV.Get());

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	ctx->RSSetViewports(1, &viewport);
}

void GBuffer::Clear(ID3D11DeviceContext* ctx, const DirectX::SimpleMath::Color& clearColor)
{
	float black[4] = {0.0f, 0.0f, 0.0f, 0.0f};

	ctx->ClearRenderTargetView(colorTargets[0].rtv.Get(), clearColor);
	ctx->ClearRenderTargetView(colorTargets[1].rtv.Get(), black);
	ctx->ClearRenderTargetView(colorTargets[2].rtv.Get(), black);
	ctx->ClearDepthStencilView(depthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

ID3D11ShaderResourceView* GBuffer::GetSRV(Attachment attachment) const
{
	if (attachment == Attachment::Depth) {
		return depthSRV.Get();
	}
	size_t idx = static_cast<size_t>(attachment);
	if (idx < colorTargets.size()) {
		return colorTargets[idx].srv.Get();
	}
	return nullptr;
}

ID3D11DepthStencilView* GBuffer::GetDSV() const
{
	return depthDSV.Get();
}

ID3D11DepthStencilView* GBuffer::GetReadOnlyDSV() const
{
	return depthReadOnlyDSV.Get();
}
