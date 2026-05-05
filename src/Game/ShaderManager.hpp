#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "GameContext.hpp"
#include "Light.hpp"
#include "Material.hpp"
#include "PerFrameConstants.hpp"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

class ShaderManager {
   public:
	enum class PassType {
		GBuffer,			  // Geometry pass
		Shadow,				  // Shadow mapping
		DeferredDirectional,  // Full-screen directional
		DeferredPoint,		  // Point light volume
		DeferredSpot,		  // Spot light volume
		Forward				  // Transparent/forward fallback
	};

	ShaderManager(GameContext* ctx);

	void Apply(PassType pass);
	void UpdateConstants(PassType pass, const void* data, size_t size);

	// Legacy compatibility
	void Apply() { Apply(PassType::Forward); }

	void UpdateConstants(const void* data) { UpdateConstants(PassType::Forward, data, sizeof(PerFrameConstants)); }

	void UpdatePerObjectConstants(const void* data, size_t size);

	void ApplyShadow();
	void UpdateShadowConstants(const Matrix& world, const Matrix& viewProj);

   private:
	struct PassData {
		ComPtr<ID3D11VertexShader> vs;
		ComPtr<ID3D11PixelShader> ps;
		ComPtr<ID3D11InputLayout> layout;
		ComPtr<ID3D11Buffer> constantBuffer;
		size_t constantSize = 0;
	};

	bool CompileShader(const wchar_t* path, const char* entry, const char* target, ComPtr<ID3DBlob>& blob);
	bool CreatePass(
		PassType type, const wchar_t* vsPath, const wchar_t* psPath, size_t constantSize,
		const D3D11_INPUT_ELEMENT_DESC* layout = nullptr, UINT layoutCount = 0
	);

	GameContext* gameContext = nullptr;
	std::unordered_map<PassType, PassData> passes;

	ComPtr<ID3D11SamplerState> samplerState;
	ComPtr<ID3D11SamplerState> pointSamplerState;
	ComPtr<ID3D11SamplerState> shadowSamplerState;

	ComPtr<ID3D11Buffer> perObjectConstantBuffer;

	// Legacy
	ComPtr<ID3D11VertexShader> shadowVertexShader;
	ComPtr<ID3D11PixelShader> shadowPixelShader;
	ComPtr<ID3D11Buffer> shadowConstantBuffer;
};
