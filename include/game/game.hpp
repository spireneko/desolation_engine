#pragma once

#include <windows.h>
#include <chrono>
#include <stdexcept>
#include <vector>

#include "game_component.hpp"
#include "players.hpp"

using namespace Microsoft::WRL;

class Game : public Gameable {
   public:
	Game(LPCSTR appName, int width, int height, HINSTANCE hInstance);
	~Game();

	void Run();

	ID3D11Device* GetDevice() const { return m_device.Get(); }

	ID3D11DeviceContext* GetContext() const { return m_context.Get(); }

	DisplayWin32* GetDisplay() { return &m_display; }

	InputDevice& GetInput() { return InputDevice::Get(); }

   private:
	void InitDirectX();
	void CreateRenderTarget();
	void Render();

	DisplayWin32 m_display;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11RenderTargetView> m_rtv;

	std::vector<std::unique_ptr<GameComponent>> m_components;
};

Game::Game(LPCSTR appName, int width, int height, HINSTANCE hInstance) : m_display(appName, width, height, hInstance)
{
	InitDirectX();
	CreateRenderTarget();

	auto player1 = std::make_unique<PlayerPad>(Players::PLAYER1);
	player1->Init(this);
	m_components.push_back(std::move(player1));

	auto player2 = std::make_unique<PlayerPad>(Players::PLAYER2);
	player2->Init(this);
	m_components.push_back(std::move(player2));
}

Game::~Game()
{
	for (auto& comp : m_components) {
		comp->Shutdown();
	}
	if (m_context) {
		m_context->ClearState();
	}
}

void Game::Run()
{
	MSG msg = {};
	auto prevTime = std::chrono::steady_clock::now();
	float totalTime = 0;
	unsigned int frameCount = 0;

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		auto curTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(curTime - prevTime).count();
		prevTime = curTime;

		totalTime += deltaTime;
		frameCount++;
		if (totalTime >= 1.0f) {
			float fps = frameCount / totalTime;
			char text[256];
			sprintf_s(text, "FPS: %.2f", fps);
			SetWindowText(m_display.GetHWND(), text);
			totalTime -= 1.0f;
			frameCount = 0;
		}

		for (auto& comp : m_components) {
			comp->Update(deltaTime);
		}

		Render();
		m_swapChain->Present(1, 0);
	}
}

void Game::InitDirectX()
{
	D3D_FEATURE_LEVEL featureLevel[] = {D3D_FEATURE_LEVEL_11_1};
	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Width = m_display.GetWidth();
	swapDesc.BufferDesc.Height = m_display.GetHeight();
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_display.GetHWND();
	swapDesc.Windowed = TRUE;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		D3D11_CREATE_DEVICE_DEBUG,
		featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapDesc,
		&m_swapChain,
		&m_device,
		nullptr,
		&m_context
	);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create device and swap chain");
	}
}

void Game::CreateRenderTarget()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to get back buffer");
	}

	hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv);
	if (FAILED(hr)) {
		throw std::runtime_error("Failed to create render target view");
	}
}

void Game::Render()
{
	m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(m_display.GetWidth());
	viewport.Height = static_cast<float>(m_display.GetHeight());
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_context->RSSetViewports(1, &viewport);

	float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);

	for (auto& comp : m_components) {
		comp->Draw();
	}
}
