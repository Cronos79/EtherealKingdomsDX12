#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "KSDevice.h"

namespace KSEngine
{
	class KSCommandAllocator
	{
	public:
		KSCommandAllocator();
		~KSCommandAllocator();

		KSCommandAllocator(const KSCommandAllocator&) = delete;
		KSCommandAllocator& operator=(const KSCommandAllocator&) = delete;

		// Initializes the command allocator using the given device and type
		bool Init(KSDevice& device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT, bool showLogging = true);

		ID3D12CommandAllocator* Get() const
		{
			return m_commandAllocator.Get();
		}

		// Resets the allocator
		bool Reset();

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	};
}