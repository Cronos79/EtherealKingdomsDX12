#include "KSSwapChain.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSSwapChain::KSSwapChain() = default;
	KSSwapChain::~KSSwapChain() = default;

	bool KSSwapChain::Init(
		KSFactory& factory,
		KSCommandQueue& commandQueue,
		HWND hwnd,
		UINT width,
		UINT height,
		UINT bufferCount,
		DXGI_FORMAT format,
		bool showLogging)
	{
		DXGI_SWAP_CHAIN_DESC1 desc = {};
		desc.BufferCount = bufferCount;
		desc.Width = width;
		desc.Height = height;
		desc.Format = format;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
		HRESULT hr = factory.Get()->CreateSwapChainForHwnd(
			commandQueue.Get(),
			hwnd,
			&desc,
			nullptr,
			nullptr,
			&swapChain1
		);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create swap chain. HRESULT: {}", hr);
			return false;
		}

		hr = swapChain1.As(&m_swapChain);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to query IDXGISwapChain3. HRESULT: {}", hr);
			return false;
		}

		if (showLogging)
		{
			LOG_INFO(L"Swap chain (IDXGISwapChain3) created successfully.");
		}
		return true;
	}
}