#include "KSFactory.h"
#include "../Logger/KSLogger.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace KSEngine
{
	KSFactory::KSFactory() = default;
	KSFactory::~KSFactory() = default;

	bool KSFactory::Init(bool enableDebug, bool showDebug)
	{
		HRESULT hr = S_OK;
#if defined(_DEBUG)
		if (enableDebug)
		{
			hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
			if (FAILED(hr))
			{
				LOG_ERROR(L"Failed to create DXGI factory with debug. HRESULT: {}", hr);
				return false;
			}
			if (showDebug)
			{
				LOG_INFO(L"DXGI factory with debug created successfully.");
			}
			
			return true;
		}
#endif
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create DXGI factory. HRESULT: {}", hr);
			return false;
		}
		if (showDebug)
		{
			LOG_INFO(L"DXGI factory created successfully.");
		}		
		return true;
	}
}