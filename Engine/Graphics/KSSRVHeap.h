#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "KSDevice.h"

namespace KSEngine
{
	class KSSRVHeap
	{
	public:
		KSSRVHeap();
		~KSSRVHeap();

		KSSRVHeap(const KSSRVHeap&) = delete;
		KSSRVHeap& operator=(const KSSRVHeap&) = delete;

		// Initializes the SRV heap
		bool Init(KSDevice& device, UINT numDescriptors = 100, bool showLogging = true);

		ID3D12DescriptorHeap* Get() const
		{
			return m_srvHeap.Get();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleStart() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleStart() const;
		UINT GetDescriptorSize(KSDevice& device) const;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	};
}