#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <array>

using Microsoft::WRL::ComPtr;

class GBuffer {
   public:
	enum class Attachment : uint8_t {
		AlbedoMetallic = 0,	 // RGB: albedo (sRGB), A: metallic
		NormalRoughness,	 // RGB: normal (world-space, packed), A: roughness
		EmissiveAO,			 // RGB: emissive, A: ambient occlusion (reserved)
		Depth,				 // D24S8 depth buffer
		Count
	};

	GBuffer() = default;
	~GBuffer() = default;

	bool Initialize(ID3D11Device* device, int width, int height);
	void Shutdown();
	void Resize(int width, int height);

	void BindForGeometryPass(ID3D11DeviceContext* ctx);
	void Clear(ID3D11DeviceContext* ctx, const DirectX::SimpleMath::Color& clearColor);

	ID3D11ShaderResourceView* GetSRV(Attachment attachment) const;
	ID3D11DepthStencilView* GetDSV() const;
	ID3D11DepthStencilView* GetReadOnlyDSV() const;

	int GetWidth() const { return width; }

	int GetHeight() const { return height; }

   private:
	struct RenderTarget {
		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11RenderTargetView> rtv;
		ComPtr<ID3D11ShaderResourceView> srv;
	};

	bool CreateAttachment(ID3D11Device* device, Attachment attachment, int w, int h);
	bool CreateDepthBuffer(ID3D11Device* device, int w, int h);

	std::array<RenderTarget, static_cast<size_t>(Attachment::Count) - 1> colorTargets;	// Without Depth
	ComPtr<ID3D11Texture2D> depthTexture;
	ComPtr<ID3D11DepthStencilView> depthDSV;
	ComPtr<ID3D11DepthStencilView> depthReadOnlyDSV;
	ComPtr<ID3D11ShaderResourceView> depthSRV;

	int width = 0;
	int height = 0;
	bool initialized = false;
};
