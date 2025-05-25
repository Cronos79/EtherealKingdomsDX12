#include "KSAdapter.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	KSAdapter::KSAdapter() = default;
	KSAdapter::~KSAdapter() = default;

	bool KSAdapter::Init(IDXGIFactory6* factory, bool highPerformance, bool showLogging)
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		HRESULT hr = E_FAIL;

		if (factory)
		{
			if (highPerformance)
			{
				// Prefer high performance GPU if available
				hr = factory->EnumAdapterByGpuPreference(
					0,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(&adapter)
				);
			}
			else
			{
				// Fallback: just get the first adapter
				hr = factory->EnumAdapters1(0, &adapter);
			}
		}

		if (FAILED(hr) || !adapter)
		{
			LOG_ERROR(L"Failed to enumerate DXGI adapter.");
			return false;
		}

		m_adapter = adapter;

		if (FAILED(m_adapter->GetDesc1(&m_desc)))
		{
			LOG_ERROR(L"Failed to get DXGI adapter description.");
			return false;
		}

		if (showLogging)
		{
			LOG_INFO(L"Selected Adapter: {}", GetDescription());
		}

		return true;
	}

	std::wstring KSAdapter::GetDescription() const
	{
		return std::wstring(m_desc.Description);
	}

	DXGI_ADAPTER_DESC1 KSAdapter::GetDescStruct() const
	{
		return m_desc;
	}
}