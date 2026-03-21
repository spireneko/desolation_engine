#pragma once

#include <Windows.h>

#include "input_device.hpp"

class DisplayWin32 {
   public:
	DisplayWin32(LPCSTR applicationName, int screenWidth, int screenHeight, HINSTANCE hInstance)
		: m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_hInstance(hInstance)
	{
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = m_hInstance;
		wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		wc.hIconSm = wc.hIcon;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = applicationName;

		RegisterClassEx(&wc);

		RECT windowRect = {0, 0, static_cast<LONG>(m_screenWidth), static_cast<LONG>(m_screenHeight)};
		AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
		DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;
		int posX = (GetSystemMetrics(SM_CXSCREEN) - m_screenWidth) / 2;
		int posY = (GetSystemMetrics(SM_CYSCREEN) - m_screenHeight) / 2;

		m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			applicationName,
			applicationName,
			dwStyle,
			posX,
			posY,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			nullptr,
			nullptr,
			m_hInstance,
			nullptr
		);

		// Регистрация RawInput для клавиатуры
		RAWINPUTDEVICE rid[1];
		rid[0].usUsagePage = 0x01;
		rid[0].usUsage = 0x06;
		rid[0].dwFlags = RIDEV_INPUTSINK;
		rid[0].hwndTarget = m_hWnd;
		if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) {
			// можно игнорировать
		}

		ShowWindow(m_hWnd, SW_SHOW);
		SetForegroundWindow(m_hWnd);
		SetFocus(m_hWnd);
		ShowCursor(true);
	}

	HWND GetHWND() const { return m_hWnd; }

	int GetWidth() const { return m_screenWidth; }

	int GetHeight() const { return m_screenHeight; }

   private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) {
			case WM_INPUT: {
				UINT dwSize = sizeof(RAWINPUT);
				static BYTE lpb[sizeof(RAWINPUT)];
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
				RAWINPUT* raw = (RAWINPUT*)lpb;
				if (raw->header.dwType == RIM_TYPEKEYBOARD) {
					InputDevice::Get().ProcessRawKeyboard(raw->data.keyboard);
				}
				return 0;
			}
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			default:
				return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	int m_screenWidth;
	int m_screenHeight;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};
