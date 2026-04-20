#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class ShadowMap {
   public:
	ShadowMap(ID3D11Device* device, int width, int height, int cascadeCount);
	~ShadowMap() = default;

	void BeginCascade(ID3D11DeviceContext* ctx, int cascadeIndex);
	void EndCascade(ID3D11DeviceContext* ctx);
	void Clear(ID3D11DeviceContext* ctx, int cascadeIndex);

	ID3D11ShaderResourceView* GetSRV() const { return srv.Get(); }

	ID3D11SamplerState* GetSampler() const { return shadowSampler.Get(); }

	int GetWidth() const { return width; }

	int GetHeight() const { return height; }

	int GetCascadeCount() const { return cascadeCount; }

   private:
	ComPtr<ID3D11Texture2D> shadowTexture;
	std::vector<ComPtr<ID3D11DepthStencilView>> dsvs;
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11SamplerState> shadowSampler;
	ComPtr<ID3D11RasterizerState> rasterizerState;

	int width, height, cascadeCount;
	D3D11_VIEWPORT viewport;
};
