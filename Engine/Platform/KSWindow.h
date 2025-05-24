#pragma once
#include "Engine/Platform/WinInclude.h"
#include <string>
#include <optional>

namespace KSEngine
{
	class KSWindow {
	public:
		KSWindow(const std::wstring& title, int width, int height);
		~KSWindow();
		KSWindow(const KSWindow&) = delete;
		KSWindow& operator=(const KSWindow&) = delete;

		HWND GetHWND() const
		{
			return m_hWnd;
		}

		static std::optional<int> ProcessMessages() noexcept;

		void Resize(int width, int height, bool fromContext = true);

	private:
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK HandleMsgRedirect(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	private:
		HWND m_hWnd = nullptr;
		std::wstring m_windowClassName;

	};
}