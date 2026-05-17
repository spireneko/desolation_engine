#pragma once

#include <SimpleMath.h>
#include <d3d11.h>
#include <memory>
#include <vector>

#include "CameraComponent.hpp"
#include "FullscreenQuad.hpp"
#include "GBuffer.hpp"
#include "GameComponent.hpp"
#include "GameContext.hpp"
#include "LightManager.hpp"
#include "ShaderManager.hpp"

using namespace DirectX::SimpleMath;

class RenderingSystem {
   public:
	RenderingSystem(GameContext* ctx);
	bool Initialize(int width, int height);
	void Shutdown();
	void Resize(int width, int height);

	void RenderFrame(
		const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, CameraComponent* camera,
		LightManager* lightManager
	);

   private:
	void ExecuteShadowPass(
		const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, const Vector3& lightDir,
		const Matrix& cameraView, float fovRadians, float aspect, float nearPlane, float farPlane
	);

	void ExecuteGBufferPass(
		const std::vector<std::shared_ptr<GameComponent>>& sceneRoots, const Matrix& view, const Matrix& proj,
		const Vector3& cameraPos, LightManager* lightManager
	);

	void ExecuteDeferredLighting(CameraComponent* camera, LightManager* lightManager);

	void ExecutePointLights(
		const std::vector<LightData::PointLight>& points, const Matrix& invViewProj, const Vector3& cameraPos,
		CameraComponent* camera
	);

	void ExecuteSpotLights(
		const std::vector<LightData::SpotLight>& spots, const Matrix& invViewProj, const Vector3& cameraPos,
		CameraComponent* camera
	);

	void ExecuteForwardPass(
		const std::vector<std::shared_ptr<GameComponent>>& transparentObjects, const Matrix& view, const Matrix& proj
	);

	void DrawSceneGeometry(const std::vector<std::shared_ptr<GameComponent>>& roots, ShaderManager::PassType pass);
	void DrawLightVolume(ID3D11DeviceContext* ctx);
	void DrawConeVolume(ID3D11DeviceContext* ctx);

	GameContext* gameContext;
	std::unique_ptr<GBuffer> gBuffer;
	std::unique_ptr<FullscreenQuad> fsQuad;

	ComPtr<ID3D11BlendState> additiveBlendState;
	ComPtr<ID3D11DepthStencilState> lightingDSS;
	ComPtr<ID3D11DepthStencilState> volumeDSS;
	ComPtr<ID3D11RasterizerState> volumeRS;

	// Light volume geometry
	ComPtr<ID3D11Buffer> sphereVertexBuffer;
	ComPtr<ID3D11Buffer> sphereIndexBuffer;
	UINT sphereIndexCount = 0;

	ComPtr<ID3D11Buffer> coneVertexBuffer;
	ComPtr<ID3D11Buffer> coneIndexBuffer;
	UINT coneIndexCount = 0;

	bool initialized = false;
};
