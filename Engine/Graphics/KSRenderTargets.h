#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "KSDevice.h"
#include "KSSwapChain.h"

namespace KSEngine
{
	class KSRenderTargets
	{
	public:
		KSRenderTargets();
		~KSRenderTargets();

		KSRenderTargets(const KSRenderTargets&) = delete;
		KSRenderTargets& operator=(const KSRenderTargets&) = delete;

		// Initializes the render targets and RTV heap
		bool Init(
			KSDevice& device,
			KSSwapChain& swapChain,
			UINT bufferCount,
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
			bool showLogging = true);

		// Accessors
		ID3D12DescriptorHeap* GetRTVHeap() const
		{
			return m_rtvHeap.Get();
		}
		ID3D12Resource* GetRenderTarget(UINT index) const
		{
			return m_renderTargets[index].Get();
		}
		UINT GetRTVDescriptorSize() const
		{
			return m_rtvDescriptorSize;
		}
		UINT GetCount() const
		{
			return static_cast<UINT>(m_renderTargets.size());
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTargets;
		UINT m_rtvDescriptorSize = 0;
	};
}