#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <wrl/client.h>

namespace Engine {

using Microsoft::WRL::ComPtr;

// Constant buffer для Vertex Shader
struct VSConstantBuffer {
	DirectX::SimpleMath::Matrix world;
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix projection;
};

// Constant buffer для Pixel Shader
struct PSConstantBuffer {
	DirectX::SimpleMath::Vector3 lightDirection;
	float padding1;
	DirectX::SimpleMath::Vector4 lightColor;
	DirectX::SimpleMath::Vector4 ambientColor;
};

template <typename T>
class ConstantBuffer {
   public:
	ConstantBuffer(ID3D11Device* device)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(T);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		device->CreateBuffer(&desc, nullptr, m_buffer.GetAddressOf());
	}

	void Update(ID3D11DeviceContext* context, const T& data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		if (SUCCEEDED(context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
			memcpy(mapped.pData, &data, sizeof(T));
			context->Unmap(m_buffer.Get(), 0);
		}
	}

	void Bind(ID3D11DeviceContext* context, UINT slot)
	{
		context->VSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
		context->PSSetConstantBuffers(slot, 1, m_buffer.GetAddressOf());
	}

   private:
	ComPtr<ID3D11Buffer> m_buffer;
};

}  // namespace Engine
