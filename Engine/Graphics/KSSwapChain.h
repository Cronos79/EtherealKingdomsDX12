#pragma once
#include <dxgi1_6.h>
#include <wrl.h>
#include "KSFactory.h"
#include "KSCommandQueue.h"

namespace KSEngine
{
	class KSSwapChain
	{
	public:
		KSSwapChain();
		~KSSwapChain();

		KSSwapChain(const KSSwapChain&) = delete;
		KSSwapChain& operator=(const KSSwapChain&) = delete;

		// Initializes the swap chain. hwnd is the window handle.
		bool Init(
			KSFactory& factory,
			KSCommandQueue& commandQueue,
			HWND hwnd,
			UINT width,
			UINT height,
			UINT bufferCount = 2,
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
			bool showLogging = true);

		IDXGISwapChain3* Get() const
		{
			return m_swapChain.Get();
		}
		UINT GetCurrentBackBufferIndex() const
		{
			return m_swapChain->GetCurrentBackBufferIndex();
		}

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	};
}