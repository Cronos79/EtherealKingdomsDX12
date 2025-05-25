#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "KSDevice.h"

namespace KSEngine
{
	class KSFence
	{
	public:
		KSFence();
		~KSFence();

		KSFence(const KSFence&) = delete;
		KSFence& operator=(const KSFence&) = delete;

		// Initializes the fence and event
		bool Init(KSDevice& device, UINT64 initialValue = 0, bool showLogging = true);

		ID3D12Fence* Get() const
		{
			return m_fence.Get();
		}
		UINT64 GetValue() const
		{
			return m_fenceValue;
		}
		void IncrementValue()
		{
			++m_fenceValue;
		}
		HANDLE GetEvent() const
		{
			return m_fenceEvent;
		}

		// Signal and wait helpers
		bool Signal(ID3D12CommandQueue* queue);
		bool Wait();

	private:
		Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fenceValue = 0;
		HANDLE m_fenceEvent = nullptr;
	};
}