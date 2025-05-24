#pragma once
#include <memory>
#include <stdint.h>
#include "Platform/KSWindow.h"
#include "Graphics/KSDirectX12.h"
#include "Platform/Keyboard.h"

namespace KSEngine
{
	class KSContext {
	public:
		// Singleton access
		static KSContext& Instance();

		// Delete copy/move
		KSContext(const KSContext&) = delete;
		KSContext& operator=(const KSContext&) = delete;
		KSContext(KSContext&&) = delete;
		KSContext& operator=(KSContext&&) = delete;

		// Window access
		KSWindow& GetWindow();
		HWND GetHWND() const;

		int32_t GetWidth() const;
		int32_t GetHeight() const;
		float GetAspectRatio() const;
		bool GetIsRunning() const;
		void StartRunning();
		void StopRunning();

		KSDirectX12& GetDirectX12();

		// Initialization
		void Initialize(const std::wstring& title, int width, int height);

		void Resize(int width, int height);

		Keyboard& GetKeyboard()
		{
			return m_keyboard;
		}
		// For convenience:
		bool KeyDown(int key) const
		{
			return m_keyboard.IsKeyDown(key);
		}
	private:
		int32_t m_width = 1920;
		int32_t m_height = 1080;
		// title of the window
		std::wstring m_title = L"Unknown";
		// If app is running
		bool m_isRunning = true;
		Keyboard m_keyboard;

	private:
		KSContext() = default;
		~KSContext() = default;

		std::unique_ptr<KSWindow> m_window;
		std::unique_ptr<KSDirectX12> m_dx12;
	};
}