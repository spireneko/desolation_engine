#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <d3d11.h>
#include <dxgi1_2.h>

// Важно для DXVK Native: используем __uuidof_var вместо __uuidof для переменных
#ifdef __uuidof_var
	#define UUIDOF_VAR(x) __uuidof_var(x)
#else
	#define UUIDOF_VAR(x) __uuidof(x)
#endif

#ifdef __linux__
	#include <cstdlib>
#endif

// DirectX объекты
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
static ID3D11Texture2D* g_pRenderTargetBuffer = nullptr;

// Размеры окна
static int g_Width = 800;
static int g_Height = 600;
static SDL_Window* g_pWindow = nullptr;

// Очистка ресурсов DirectX
void CleanupDirectX()
{
	if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}
	if (g_pRenderTargetBuffer) {
		g_pRenderTargetBuffer->Release();
		g_pRenderTargetBuffer = nullptr;
	}
	if (g_pSwapChain) {
		g_pSwapChain->Release();
		g_pSwapChain = nullptr;
	}
	if (g_pDeviceContext) {
		g_pDeviceContext->Release();
		g_pDeviceContext = nullptr;
	}
	if (g_pDevice) {
		g_pDevice->Release();
		g_pDevice = nullptr;
	}
}

// Инициализация DirectX 11 через DXVK Native
bool InitializeDirectX()
{
	// 1. Create Device & DeviceContext
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	uint32_t createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL chosenFeatureLevel;

	HRESULT result = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&g_pDevice,
		&chosenFeatureLevel,
		&g_pDeviceContext
	);

	if (FAILED(result)) {
		SDL_Log("D3D11CreateDevice failed: 0x%08X", (unsigned int)result);
		return false;
	}

	SDL_Log("D3D11 Device created, feature level: 0x%X", chosenFeatureLevel);

	// 2. Получаем DXGI Factory из устройства
	IDXGIDevice1* pDXGIDevice = nullptr;
	result = g_pDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&pDXGIDevice));
	if (FAILED(result)) {
		SDL_Log("QueryInterface IDXGIDevice1 failed: 0x%08X", (unsigned int)result);
		return false;
	}

	IDXGIAdapter* pDXGIAdapter = nullptr;
	result = pDXGIDevice->GetAdapter(&pDXGIAdapter);
	pDXGIDevice->Release();

	if (FAILED(result)) {
		SDL_Log("GetAdapter failed: 0x%08X", (unsigned int)result);
		return false;
	}

	IDXGIFactory2* pDXGIFactory = nullptr;
	result = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&pDXGIFactory));
	pDXGIAdapter->Release();

	if (FAILED(result)) {
		SDL_Log("GetParent (IDXGIFactory2) failed: 0x%08X", (unsigned int)result);
		return false;
	}

	SDL_Log("DXGI Factory obtained from device");

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferDesc.Width = g_Width;
	swapChainDesc.BufferDesc.Height = g_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
#ifdef WIN32
	HWND hwnd =
		(HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(g_pWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	swapChainDesc.OutputWindow = hwnd;
#else
	swapChainDesc.OutputWindow = g_pWindow;
#endif
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

	result = pDXGIFactory->CreateSwapChain(g_pDevice, &swapChainDesc, &g_pSwapChain);
	pDXGIFactory->Release();

	if (FAILED(result)) {
		SDL_Log("CreateSwapChain failed: 0x%08X", (unsigned int)result);
		return false;
	}

	SDL_Log("Swap chain created");

	// 4. Create RenderTargetView
	result = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&g_pRenderTargetBuffer));
	if (FAILED(result)) {
		SDL_Log("GetBuffer failed: 0x%08X", (unsigned int)result);
		return false;
	}

	result = g_pDevice->CreateRenderTargetView(g_pRenderTargetBuffer, nullptr, &g_pRenderTargetView);
	if (FAILED(result)) {
		SDL_Log("CreateRenderTargetView failed: 0x%08X", (unsigned int)result);
		return false;
	}

	// 5. Bind RTV to Output Merger
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	// 6. Set Viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(g_Width);
	viewport.Height = static_cast<float>(g_Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_pDeviceContext->RSSetViewports(1, &viewport);

	SDL_Log("DirectX 11 initialized successfully!");
	return true;
}

// Отрисовка кадра
void Render()
{
	if (!g_pDeviceContext || !g_pRenderTargetView) {
		return;
	}

	// Очищаем экран (темно-синий)
	const float clearColor[4] = {0.5f, 0.2f, 0.4f, 1.0f};
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

	// Показываем кадр
	g_pSwapChain->Present(1, 0);
}

// SDL Callback: Инициализация приложения
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
// Важно для работы dxvk
#ifdef __linux__
	setenv("DXVK_WSI_DRIVER", "SDL3", 1);
#endif

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Log("Video driver: %s", SDL_GetCurrentVideoDriver());

	// Создаем окно
	g_pWindow = SDL_CreateWindow("Desolation Engine", g_Width, g_Height, SDL_WINDOW_RESIZABLE);
	if (!g_pWindow) {
		SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Получаем реальный размер окна
	SDL_GetWindowSize(g_pWindow, &g_Width, &g_Height);
	SDL_Log("Window created: %dx%d", g_Width, g_Height);

	// Инициализируем DirectX
	if (!InitializeDirectX()) {
		SDL_Log("DirectX initialization failed!");
		return SDL_APP_FAILURE;
	}

	HRESULT hr = g_pSwapChain->Present(1, 0);
	if (FAILED(hr)) {
		SDL_Log("Present failed: 0x%08X", (unsigned int)hr);
	}

	*appstate = g_pWindow;
	return SDL_APP_CONTINUE;
}

// SDL Callback: Обработка событий
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;
	}

	// Обработка изменения размера окна
	if (event->type == SDL_EVENT_WINDOW_RESIZED) {
		SDL_Log("Window resized to %dx%d", event->window.data1, event->window.data2);
		g_Width = event->window.data1;
		g_Height = event->window.data2;

		// Resize swapchain
		if (g_pSwapChain) {
			if (g_pRenderTargetView) {
				g_pRenderTargetView->Release();
				g_pRenderTargetView = nullptr;
			}
			if (g_pRenderTargetBuffer) {
				g_pRenderTargetBuffer->Release();
				g_pRenderTargetBuffer = nullptr;
			}

			HRESULT result = g_pSwapChain->ResizeBuffers(1, g_Width, g_Height, DXGI_FORMAT_UNKNOWN, 0);
			if (SUCCEEDED(result)) {
				result = g_pSwapChain->GetBuffer(
					0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&g_pRenderTargetBuffer)
				);
				if (SUCCEEDED(result)) {
					g_pDevice->CreateRenderTargetView(g_pRenderTargetBuffer, nullptr, &g_pRenderTargetView);
					g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

					D3D11_VIEWPORT viewport = {};
					viewport.TopLeftX = 0.0f;
					viewport.TopLeftY = 0.0f;
					viewport.Width = static_cast<float>(g_Width);
					viewport.Height = static_cast<float>(g_Height);
					viewport.MinDepth = 0.0f;
					viewport.MaxDepth = 1.0f;
					g_pDeviceContext->RSSetViewports(1, &viewport);
				}
			}
		}
	}

	return SDL_APP_CONTINUE;
}

// SDL Callback: Отрисовка кадра
SDL_AppResult SDL_AppIterate(void* appstate)
{
	Render();
	return SDL_APP_CONTINUE;
}

// SDL Callback: Завершение приложения
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	CleanupDirectX();

	if (g_pWindow) {
		SDL_DestroyWindow(g_pWindow);
		g_pWindow = nullptr;
	}

	SDL_Quit();
}
