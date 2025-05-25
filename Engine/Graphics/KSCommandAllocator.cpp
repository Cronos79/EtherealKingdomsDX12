#include "KSCommandAllocator.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSCommandAllocator::KSCommandAllocator() = default;
	KSCommandAllocator::~KSCommandAllocator() = default;

	bool KSCommandAllocator::Init(KSDevice& device, D3D12_COMMAND_LIST_TYPE type, bool showLogging)
	{
		HRESULT hr = device.Get()->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create D3D12 command allocator. HRESULT: {}", hr);
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"D3D12 command allocator created successfully.");
		}
		return true;
	}

	bool KSCommandAllocator::Reset()
	{
		HRESULT hr = m_commandAllocator->Reset();
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to reset D3D12 command allocator. HRESULT: {}", hr);
			return false;
		}
		return true;
	}
}