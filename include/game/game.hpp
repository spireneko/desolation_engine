#pragma once

#include <windows.h>
#include <chrono>
#include <stdexcept>
#include <vector>

#include "ball.hpp"
#include "display.hpp"
#include "divider.hpp"
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

	int GetScorePlayer1() const { return m_player1Score; }

	int GetScorePlayer2() const { return m_player2Score; }

	void AddScorePlayer1() { m_player1Score++; }

	void AddScorePlayer2() { m_player2Score++; }

	void ResetScores()
	{
		m_player1Score = 0;
		m_player2Score = 0;
	}

	const std::vector<std::unique_ptr<GameComponent>>& GetComponents() const { return m_components; }

	void Restart() override;

   private:
	void InitDirectX();
	void CreateRenderTarget();
	void Render();

	int m_player1Score;
	int m_player2Score;

	DisplayWin32 m_display;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11RenderTargetView> m_rtv;

	std::vector<std::unique_ptr<GameComponent>> m_components;

	std::chrono::steady_clock::time_point m_startTime;
	std::chrono::steady_clock::time_point m_prevTime;
	float m_totalTime = 0.0;
};

Game::Game(LPCSTR appName, int width, int height, HINSTANCE hInstance)
	: m_display(appName, width, height, hInstance), m_player1Score(0), m_player2Score(0)
{
	InitDirectX();
	CreateRenderTarget();

	auto player1 = std::make_unique<PlayerPad>(Players::PLAYER1);
	player1->Init(this);
	m_components.push_back(std::move(player1));

	auto player2 = std::make_unique<PlayerPad>(Players::PLAYER2);
	player2->Init(this);
	m_components.push_back(std::move(player2));

	const int dividersAmount = 18;
	for (int i = 0; i < dividersAmount; ++i) {
		float yPos = (i + 1) * (2.0 / (float)(dividersAmount + 1)) - 1.0;

		auto divider = std::make_unique<Divider>(Vector3(0.0, yPos, 0.0));
		divider->Init(this);
		m_components.push_back(std::move(divider));
	}

	auto ball = std::make_unique<Ball>(Vector3(0.0, 0.0, 0.0), Vector3(-1.0, 0.0, 0.0));
	ball->Init(this);
	m_components.push_back(std::move(ball));
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
	m_prevTime = std::chrono::steady_clock::now();

	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) {
			break;
		}

		auto currTime = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(currTime - m_prevTime).count();
		m_prevTime = currTime;

		m_totalTime += deltaTime;

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

void Game::Restart()
{
	for (auto& comp : m_components) {
		comp->Restart();
	}

	std::cout << m_player1Score << ":" << m_player2Score << "\n";
}
