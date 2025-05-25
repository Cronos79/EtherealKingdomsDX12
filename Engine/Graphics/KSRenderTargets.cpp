#include "KSRenderTargets.h"
#include "../Logger/KSLogger.h"
#include <d3dx12.h>

namespace KSEngine
{
	KSRenderTargets::KSRenderTargets() = default;
	KSRenderTargets::~KSRenderTargets() = default;

	bool KSRenderTargets::Init(
		KSDevice& device,
		KSSwapChain& swapChain,
		UINT bufferCount,
		DXGI_FORMAT format,
		bool showLogging)
	{
		m_rtvDescriptorSize = device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = bufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(device.Get()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
		{
			LOG_ERROR(L"Failed to create RTV descriptor heap.");
			return false;
		}

		m_renderTargets.resize(bufferCount);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < bufferCount; ++i)
		{
			if (FAILED(swapChain.Get()->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
			{
				LOG_ERROR(L"Failed to get swap chain buffer {}.", i);
				return false;
			}
			device.Get()->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

		if (showLogging)
		{
			LOG_INFO(L"Render targets and RTV heap created successfully.");
		}
		return true;
	}
}