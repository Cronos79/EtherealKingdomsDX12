#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "KSDevice.h"

namespace KSEngine
{
	class KSCommandQueue
	{
	public:
		KSCommandQueue();
		~KSCommandQueue();

		KSCommandQueue(const KSCommandQueue&) = delete;
		KSCommandQueue& operator=(const KSCommandQueue&) = delete;

		// Initializes the command queue using the given device
		bool Init(KSDevice& device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT, bool showLogging = true);

		ID3D12CommandQueue* Get() const
		{
			return m_commandQueue.Get();
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	};
}