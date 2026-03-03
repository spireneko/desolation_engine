#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

#include <d3d11.h>
#include <dxgi1_2.h>

#include <array>

#include "shader.hpp"

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

// После существующих глобальных объектов DirectX
static ID3D11VertexShader* g_pVS = nullptr;
static ID3D11PixelShader* g_pPS = nullptr;
static ID3D11InputLayout* g_pInputLayout = nullptr;
static ID3D11Buffer* g_pVertexBuffer = nullptr;

// Размеры окна
static int g_Width = 750;
static int g_Height = 750;
static SDL_Window* g_pWindow = nullptr;

// Структура для хранения скомпилированных шейдеров
struct CompiledShaders {
	Slang::ComPtr<slang::IBlob> vertexShader;
	Slang::ComPtr<slang::IBlob> pixelShader;
	Slang::ComPtr<slang::IComponentType> linkedProgram;
};

// Структура вершины (должна совпадать с VSIn в шейдере)
struct SimpleVertex {
	float x, y, z, w;  // POSITION0
	float r, g, b, a;  // COLOR0
};

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
	if (g_pVertexBuffer) {
		g_pVertexBuffer->Release();
		g_pVertexBuffer = nullptr;
	}
	if (g_pInputLayout) {
		g_pInputLayout->Release();
		g_pInputLayout = nullptr;
	}
	if (g_pPS) {
		g_pPS->Release();
		g_pPS = nullptr;
	}
	if (g_pVS) {
		g_pVS->Release();
		g_pVS = nullptr;
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
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

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

	const float clearColor[4] = {0.1f, 0.1f, 0.15f, 1.0f};
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);

	// ────────────── Здесь начинается отрисовка ──────────────

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pDeviceContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	g_pDeviceContext->IASetInputLayout(g_pInputLayout);
	g_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_pDeviceContext->VSSetShader(g_pVS, nullptr, 0);
	g_pDeviceContext->PSSetShader(g_pPS, nullptr, 0);

	g_pDeviceContext->Draw(3, 0);  // 3 вершины → один треугольник

	// ────────────────────────────────────────────────────────

	g_pSwapChain->Present(1, 0);
}

void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
{
	if (diagnosticsBlob) {
		const char* diagnostics = (const char*)diagnosticsBlob->getBufferPointer();
		if (diagnostics && strlen(diagnostics) > 0) {
			SDL_Log("Slang diagnostics: %s", diagnostics);
		}
	}
}

bool SlangCompile(CompiledShaders& outShaders)
{
	Slang::ComPtr<slang::IGlobalSession> globalSession;
	createGlobalSession(globalSession.writeRef());

	slang::SessionDesc sessionDesc = {};

	const char* searchPaths[] = {"shaders/"};
	sessionDesc.searchPathCount = 1;
	sessionDesc.searchPaths = searchPaths;

	slang::TargetDesc targetDesc = {};
	targetDesc.format = SLANG_DXBC;
	targetDesc.profile = globalSession->findProfile("sm_5_0");

	sessionDesc.targets = &targetDesc;
	sessionDesc.targetCount = 1;

	Slang::ComPtr<slang::ISession> session;
	globalSession->createSession(sessionDesc, session.writeRef());

	Slang::ComPtr<slang::IModule> slangModule;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		const char* moduleName = "MyVeryFirstShader";
		slangModule = session->loadModule(moduleName, diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		if (!slangModule) {
			SDL_Log("Failed to load module: %s", moduleName);
			return false;
		}
	}

	// Находим функции шейдеров в модуле
	Slang::ComPtr<slang::IEntryPoint> vertexEntryPoint;
	Slang::ComPtr<slang::IEntryPoint> pixelEntryPoint;

	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		slangModule->findEntryPointByName("vsMain", vertexEntryPoint.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		if (!vertexEntryPoint) {
			SDL_Log("Failed to find vertex entry point");
			return false;
		}
	}

	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		slangModule->findEntryPointByName("psMain", pixelEntryPoint.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		if (!pixelEntryPoint) {
			SDL_Log("Failed to find fragment entry point");
			return false;
		}
	}

	std::array<slang::IComponentType*, 3> componentTypes = {slangModule, vertexEntryPoint, pixelEntryPoint};

	Slang::ComPtr<slang::IComponentType> composedProgram;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult result = session->createCompositeComponentType(
			componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnosticsBlob.writeRef()
		);
		diagnoseIfNeeded(diagnosticsBlob);

		if (!composedProgram) {
			SDL_Log("Failed to create composite program");
			return false;
		}
	}

	Slang::ComPtr<slang::IComponentType> linkedProgram;
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
		diagnoseIfNeeded(diagnosticsBlob);
		SLANG_RETURN_ON_FAIL(result);
	}

	// Получаем скомпилированный код шейдеров(DXBC)
	// Индекс 0 = vertexEntryPoint, Индекс 1 = pixelEntryPoint (в порядке добавления)
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob;

		// Vertex Shader (entry point index 0)
		SlangResult result = linkedProgram->getEntryPointCode(
			0,	// entryPointIndex - vsMain был добавлен первым
			0,	// targetIndex
			outShaders.vertexShader.writeRef(),
			diagnosticsBlob.writeRef()
		);
		diagnoseIfNeeded(diagnosticsBlob);
		if (!composedProgram) {
			SDL_Log("Failed to get vertex shader");
			return false;
		}

		// Pixel Shader (entry point index 1)
		result = linkedProgram->getEntryPointCode(
			1,	// entryPointIndex - psMain был добавлен вторым
			0,	// targetIndex
			outShaders.pixelShader.writeRef(),
			diagnosticsBlob.writeRef()
		);
		diagnoseIfNeeded(diagnosticsBlob);
		if (!composedProgram) {
			SDL_Log("Failed to get fragment shader");
			return false;
		}
	}

	// Сохраняем linkedProgram для возможного использования reflection
	outShaders.linkedProgram = linkedProgram;

	SDL_Log("Slang compilation successful!");
	SDL_Log("Vertex shader size: %zu bytes", outShaders.vertexShader->getBufferSize());
	SDL_Log("Pixel shader size: %zu bytes", outShaders.pixelShader->getBufferSize());

	return true;
}

bool CreateShaderObjects(const CompiledShaders& shaders)
{
	HRESULT hr;

	// Vertex Shader
	hr = g_pDevice->CreateVertexShader(
		shaders.vertexShader->getBufferPointer(), shaders.vertexShader->getBufferSize(), nullptr, &g_pVS
	);
	if (FAILED(hr)) {
		SDL_Log("CreateVertexShader failed: 0x%08X", (unsigned)hr);
		return false;
	}

	// Pixel Shader
	hr = g_pDevice->CreatePixelShader(
		shaders.pixelShader->getBufferPointer(), shaders.pixelShader->getBufferSize(), nullptr, &g_pPS
	);
	if (FAILED(hr)) {
		SDL_Log("CreatePixelShader failed: 0x%08X", (unsigned)hr);
		return false;
	}

	// Input Layout — описание, как читать вершины
	std::array<D3D11_INPUT_ELEMENT_DESC, 2> layout{{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	}};

	hr = g_pDevice->CreateInputLayout(
		layout.data(),
		layout.size(),
		shaders.vertexShader->getBufferPointer(),  // reflection берётся из VS байткода
		shaders.vertexShader->getBufferSize(),
		&g_pInputLayout
	);
	if (FAILED(hr)) {
		SDL_Log("CreateInputLayout failed: 0x%08X", (unsigned)hr);
		return false;
	}

	SDL_Log("Shaders and input layout created successfully");
	return true;
}

bool CreateTriangle()
{
	SimpleVertex vertices[] = {
		{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},	 // красный
		{0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},	 // зелёный
		{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f},	 // синий
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	HRESULT hr = g_pDevice->CreateBuffer(&bd, &initData, &g_pVertexBuffer);
	if (FAILED(hr)) {
		SDL_Log("CreateBuffer failed: 0x%08X", (unsigned)hr);
		return false;
	}

	return true;
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

	// CompiledShaders shaders = {};
	// if (!SlangCompile(shaders)) {
	// 	SDL_Log("Slang compilation failed!");
	// 	return SDL_APP_FAILURE;
	// }

	// if (!CreateShaderObjects(shaders)) {
	// 	return SDL_APP_FAILURE;
	// }

	// if (!CreateTriangle()) {
	// 	return SDL_APP_FAILURE;
	// }

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
