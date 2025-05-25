#include "KSFence.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSFence::KSFence() = default;
	KSFence::~KSFence()
	{
		if (m_fenceEvent)
		{
			CloseHandle(m_fenceEvent);
			m_fenceEvent = nullptr;
		}
	}

	bool KSFence::Init(KSDevice& device, UINT64 initialValue, bool showLogging)
	{
		HRESULT hr = device.Get()->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create fence. HRESULT: {}", hr);
			return false;
		}
		m_fenceValue = initialValue + 1;
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent)
		{
			LOG_ERROR(L"Failed to create fence event.");
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"Fence and event created successfully.");
		}
		return true;
	}

	bool KSFence::Signal(ID3D12CommandQueue* queue)
	{
		HRESULT hr = queue->Signal(m_fence.Get(), m_fenceValue);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to signal fence. HRESULT: {}", hr);
			return false;
		}
		return true;
	}

	bool KSFence::Wait()
	{
		if (m_fence->GetCompletedValue() < m_fenceValue)
		{
			m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
		return true;
	}
}