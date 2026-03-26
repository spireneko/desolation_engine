#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

namespace Engine {

using Microsoft::WRL::ComPtr;

class Shader {
   public:
	Shader() = default;
	~Shader() = default;

	bool Load(ID3D11Device* device, const std::string& vsPath, const std::string& psPath)
	{
		// Загрузка скомпилированных шейдеров из файлов
		// Для простоты - заглушка, реальная реализация использует D3DReadFileToBlob
		return false;
	}

	void SetVertexShader(ComPtr<ID3D11VertexShader> vs) { m_vertexShader = vs; }

	void SetPixelShader(ComPtr<ID3D11PixelShader> ps) { m_pixelShader = ps; }

	ComPtr<ID3D11VertexShader>& GetVertexShader() { return m_vertexShader; }

	ComPtr<ID3D11PixelShader>& GetPixelShader() { return m_pixelShader; }

   private:
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
};

}  // namespace Engine
