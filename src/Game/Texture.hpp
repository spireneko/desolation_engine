#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <string>

#include "GameContext.hpp"

using Microsoft::WRL::ComPtr;

class Texture {
   public:
	Texture() = default;
	~Texture() = default;

	bool LoadFromFile(GameContext* context, const std::wstring& filePath);
	ID3D11ShaderResourceView* GetView() const;
	static std::shared_ptr<Texture> CreateWhite(GameContext* context);

   private:
	ComPtr<ID3D11ShaderResourceView> textureView;
	static ComPtr<ID3D11ShaderResourceView> CreateWhiteView(ID3D11Device* device);
};
