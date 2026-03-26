#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

#include "../Graphics/Camera.hpp"
#include "../Graphics/ConstantBuffers.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Shader.hpp"

namespace Engine {

using Microsoft::WRL::ComPtr;
using namespace DirectX::SimpleMath;

class Renderer {
   public:
	Renderer() = default;
	~Renderer() = default;

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	bool Initialize(HWND hwnd, int width, int height)
	{
		m_width = width;
		m_height = height;

		// Создание устройства и swap chain
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL featureLevel;
		HRESULT hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			m_swapChain.GetAddressOf(),
			m_device.GetAddressOf(),
			&featureLevel,
			m_context.GetAddressOf()
		);

		if (FAILED(hr)) {
			return false;
		}

		// Создание render target view
		ComPtr<ID3D11Texture2D> backBuffer;
		hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		if (FAILED(hr)) {
			return false;
		}

		hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// Создание depth stencil buffer
		CreateDepthStencilBuffer(width, height);

		// Настройка viewport
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(width);
		viewport.Height = static_cast<float>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		m_context->RSSetViewports(1, &viewport);

		// Загрузка шейдеров
		if (!m_shader.Load(m_device.Get(), "shaders/VertexShader.cso", "shaders/PixelShader.cso")) {
			// Компиляция при первом запуске
			if (!CompileAndLoadShaders()) {
				return false;
			}
		}

		// Создание constant buffers
		m_vsConstantBuffer = std::make_unique<ConstantBuffer<VSConstantBuffer>>(m_device.Get());
		m_psConstantBuffer = std::make_unique<ConstantBuffer<PSConstantBuffer>>(m_device.Get());

		// Настройка rasterizer state
		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthClipEnable = true;

		hr = m_device->CreateRasterizerState(&rasterDesc, m_rasterizerState.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		m_context->RSSetState(m_rasterizerState.Get());

		return true;
	}

	void CreateDepthStencilBuffer(int width, int height)
	{
		D3D11_TEXTURE2D_DESC depthDesc = {};
		depthDesc.Width = width;
		depthDesc.Height = height;
		depthDesc.MipLevels = 1;
		depthDesc.ArraySize = 1;
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.Usage = D3D11_USAGE_DEFAULT;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		ComPtr<ID3D11Texture2D> depthStencilBuffer;
		m_device->CreateTexture2D(&depthDesc, nullptr, depthStencilBuffer.GetAddressOf());
		m_device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, m_depthStencilView.GetAddressOf());

		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		ComPtr<ID3D11DepthStencilState> dsState;
		m_device->CreateDepthStencilState(&dsDesc, dsState.GetAddressOf());
		m_context->OMSetDepthStencilState(dsState.Get(), 0);
	}

	bool CompileAndLoadShaders()
	{
		// Простая компиляция шейдеров через систему
		// В реальном проекте используйте pre-build step
		HRESULT hr = D3DCompileFromFile(
			L"shaders/VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, m_vsBlob.GetAddressOf(), nullptr
		);

		if (FAILED(hr)) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile vertex shader: %s", hr);
			return false;
		}

		hr = D3DCompileFromFile(
			L"shaders/PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, m_psBlob.GetAddressOf(), nullptr
		);

		if (FAILED(hr)) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile pixel shader: %s", hr);
			return false;
		}

		m_device->CreateVertexShader(
			m_vsBlob->GetBufferPointer(), m_vsBlob->GetBufferSize(), nullptr, m_shader.GetVertexShader().GetAddressOf()
		);

		m_device->CreatePixelShader(
			m_psBlob->GetBufferPointer(), m_psBlob->GetBufferSize(), nullptr, m_shader.GetPixelShader().GetAddressOf()
		);

		// Создание input layout
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		m_device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			m_vsBlob->GetBufferPointer(),
			m_vsBlob->GetBufferSize(),
			m_inputLayout.GetAddressOf()
		);

		return true;
	}

	void Resize(int width, int height)
	{
		if (width == 0 || height == 0) {
			return;
		}

		m_context->OMSetRenderTargets(0, nullptr, nullptr);
		m_renderTargetView.Reset();
		m_depthStencilView.Reset();

		m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		ComPtr<ID3D11Texture2D> backBuffer;
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
		m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());

		CreateDepthStencilBuffer(width, height);

		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(width);
		viewport.Height = static_cast<float>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		m_context->RSSetViewports(1, &viewport);

		m_width = width;
		m_height = height;
	}

	void BeginFrame()
	{
		float color[4] = {0.3f, 0.3f, 0.3f, 1.0f};
		m_context->ClearRenderTargetView(m_renderTargetView.Get(), color);
		m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->VSSetShader(m_shader.GetVertexShader().Get(), nullptr, 0);
		m_context->PSSetShader(m_shader.GetPixelShader().Get(), nullptr, 0);
	}

	void EndFrame() { m_swapChain->Present(1, 0); }

	// Методы для рендеринга объектов из систем
	void SetCameraConstants(const Matrix& view, const Matrix& projection)
	{
		m_viewMatrix = view;
		m_projectionMatrix = projection;
	}

	void DrawMesh(const Mesh& mesh, const Matrix& world)
	{
		VSConstantBuffer vsData;
		vsData.world = world.Transpose();
		vsData.view = m_viewMatrix.Transpose();
		vsData.projection = m_projectionMatrix.Transpose();

		m_vsConstantBuffer->Update(m_context.Get(), vsData);
		m_vsConstantBuffer->Bind(m_context.Get(), 0);

		PSConstantBuffer psData = {};
		psData.lightDirection = Vector3(0.5f, -1.0f, 0.5f);
		psData.lightDirection.Normalize();
		psData.lightColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		psData.ambientColor = Vector4(0.2f, 0.2f, 0.2f, 1.0f);

		m_psConstantBuffer->Update(m_context.Get(), psData);
		m_psConstantBuffer->Bind(m_context.Get(), 0);

		mesh.Bind(m_context.Get());
		mesh.Draw(m_context.Get());
	}

	ID3D11Device* GetDevice() const { return m_device.Get(); }

	ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

   private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	ComPtr<ID3D11RasterizerState> m_rasterizerState;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3DBlob> m_vsBlob;
	ComPtr<ID3DBlob> m_psBlob;

	Shader m_shader;
	std::unique_ptr<ConstantBuffer<VSConstantBuffer>> m_vsConstantBuffer;
	std::unique_ptr<ConstantBuffer<PSConstantBuffer>> m_psConstantBuffer;

	Matrix m_viewMatrix = Matrix::Identity;
	Matrix m_projectionMatrix = Matrix::Identity;

	int m_width = 0;
	int m_height = 0;
};

}  // namespace Engine
