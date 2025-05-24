#include "KSCommandQueue.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSCommandQueue::KSCommandQueue() = default;
	KSCommandQueue::~KSCommandQueue() = default;

	bool KSCommandQueue::Init(KSDevice& device, D3D12_COMMAND_LIST_TYPE type, bool showLogging)
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = type;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		queueDesc.NodeMask = 0;

		HRESULT hr = device.Get()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create D3D12 command queue. HRESULT: {}", hr);
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"D3D12 command queue created successfully.");
		}
		return true;
	}
}