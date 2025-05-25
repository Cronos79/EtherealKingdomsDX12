#include "KSCommandList.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSCommandList::KSCommandList() = default;
	KSCommandList::~KSCommandList() = default;

	bool KSCommandList::Init(
		KSDevice& device,
		KSCommandAllocator& allocator,
		D3D12_COMMAND_LIST_TYPE type,
		bool showLogging)
	{
		HRESULT hr = device.Get()->CreateCommandList(
			0,
			type,
			allocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_commandList)
		);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create D3D12 command list. HRESULT: {}", hr);
			return false;
		}
		// Command lists are created in the recording state, so close it initially
		hr = m_commandList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to close D3D12 command list after creation. HRESULT: {}", hr);
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"D3D12 command list created successfully.");
		}
		return true;
	}

	bool KSCommandList::Reset(KSCommandAllocator& allocator, ID3D12PipelineState* pso)
	{
		HRESULT hr = m_commandList->Reset(allocator.Get(), pso);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to reset D3D12 command list. HRESULT: {}", hr);
			return false;
		}
		return true;
	}

	bool KSCommandList::Close()
	{
		HRESULT hr = m_commandList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to close D3D12 command list. HRESULT: {}", hr);
			return false;
		}
		return true;
	}
}