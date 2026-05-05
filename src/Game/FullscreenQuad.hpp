#pragma once

#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class FullscreenQuad {
   public:
	bool Initialize(ID3D11Device* device);
	void Draw(ID3D11DeviceContext* ctx);

   private:
	ComPtr<ID3D11Buffer> vertexBuffer;
};
