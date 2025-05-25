#include "KSSRVHeap.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSSRVHeap::KSSRVHeap() = default;
	KSSRVHeap::~KSSRVHeap() = default;

	bool KSSRVHeap::Init(KSDevice& device, UINT numDescriptors, bool showLogging)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = numDescriptors;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = 0;

		HRESULT hr = device.Get()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create SRV descriptor heap. HRESULT: {}", hr);
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"SRV descriptor heap created successfully.");
		}
		return true;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE KSSRVHeap::GetCPUHandleStart() const
	{
		return m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE KSSRVHeap::GetGPUHandleStart() const
	{
		return m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	}

	UINT KSSRVHeap::GetDescriptorSize(KSDevice& device) const
	{
		return device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}