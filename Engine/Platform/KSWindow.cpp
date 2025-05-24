#include "Engine/Platform/KSWindow.h"
#include "../KSContext.h"
#include "../Logger/KSLogger.h"
#include "imgui_impl_win32.h"

KSWindow::KSWindow(const std::wstring& title, int width, int height)
	: m_windowClassName(L"KSWindowClass")
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = m_windowClassName.c_str();
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);

	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
	if (AdjustWindowRect(&wr, style, FALSE) == 0)
	{
		LOG_ERROR(L"AdjustWindowRect failed: {}", GetLastError());
	}
	// create window & get hWnd
	m_hWnd = CreateWindow(
		m_windowClassName.c_str(), title.c_str(),
		style,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, GetModuleHandle(nullptr), this
	);

	if (m_hWnd)
	{
		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	}
	else
	{
		LOG_ERROR(L"CreateWindow failed: {}", GetLastError());
	}
	LOG_INFO(L"Window created: {} ({}x{})", title, width, height);
}

KSWindow::~KSWindow()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		UnregisterClass(m_windowClassName.c_str(), GetModuleHandle(nullptr));
	}
}

std::optional<int> KSWindow::ProcessMessages() noexcept
{
	MSG msg;
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// return empty optional when not quitting app
	return {};
}

void KSWindow::Resize(int width, int height, bool fromContext)
{
	if (fromContext) return;
	// Resize the window
	if (m_hWnd)
	{
		RECT wr;
		wr.left = 0;
		wr.right = width;
		wr.top = 0;
		wr.bottom = height;
		DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
		if (AdjustWindowRect(&wr, style, FALSE) == 0)
		{
			// log me
		}
		SetWindowPos(m_hWnd, nullptr, 0, 0, wr.right - wr.left, wr.bottom - wr.top, SWP_NOMOVE | SWP_NOZORDER);
	}
}

LRESULT CALLBACK KSWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
	if (msg == WM_NCCREATE)
	{
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		KSWindow* const pWnd = static_cast<KSWindow*>(pCreate->lpCreateParams);
		// set WinAPI-managed user data to store ptr to window instance
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&KSWindow::HandleMsgRedirect));
		// forward message to window instance handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
	// if we get a message before the WM_NCCREATE message, handle with default handler
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK KSWindow::HandleMsgRedirect(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// retrieve ptr to window instance
	KSWindow* const pWnd = reinterpret_cast<KSWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	// forward message to window instance handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT KSWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			// log me
		}
		break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		if (width != 0 && height != 0)
		{
			KSContext::Instance().Resize(width, height);
		}
		break;
	}
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

