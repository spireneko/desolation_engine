#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

class ShaderManager {
   public:
	struct ShaderBytecode {
		const BYTE* data;
		size_t size;
	};

	bool Initialize(ID3D11Device* device);

	void Apply(ID3D11DeviceContext* context);
	void UpdateConstants(ID3D11DeviceContext* context, const void* data, size_t size);

   private:
	bool CompileShaders();

	ComPtr<ID3D11VertexShader> vertexShader;
	ComPtr<ID3D11PixelShader> pixelShader;
	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> constantBuffer;

	std::vector<BYTE> vsBlob;
	std::vector<BYTE> psBlob;
};
