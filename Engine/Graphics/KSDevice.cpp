#include "KSDevice.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSDevice::KSDevice() = default;

	bool KSDevice::Init(IDXGIAdapter1* adapter /*= nullptr*/, bool showLogging /*= true*/)
	{
		HRESULT hr = D3D12CreateDevice(
			adapter,
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&m_device)
		);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create D3D12 device. HRESULT: {}", hr);
			return false;
		}
		if (showLogging)
		{
			LOG_INFO(L"D3D12 device created successfully.");
		}
		return true;
	}

	KSDevice::~KSDevice() = default;

	
}