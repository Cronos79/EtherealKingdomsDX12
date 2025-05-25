#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "KSDevice.h"
#include "KSCommandAllocator.h"

namespace KSEngine
{
	class KSCommandList
	{
	public:
		KSCommandList();
		~KSCommandList();

		KSCommandList(const KSCommandList&) = delete;
		KSCommandList& operator=(const KSCommandList&) = delete;

		// Initializes the command list using the given device and allocator
		bool Init(
			KSDevice& device,
			KSCommandAllocator& allocator,
			D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			bool showLogging = true);

		ID3D12GraphicsCommandList* Get() const
		{
			return m_commandList.Get();
		}

		// Resets the command list with the given allocator and optional initial PSO
		bool Reset(KSCommandAllocator& allocator, ID3D12PipelineState* pso = nullptr);

		// Closes the command list
		bool Close();

	private:
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	};
}