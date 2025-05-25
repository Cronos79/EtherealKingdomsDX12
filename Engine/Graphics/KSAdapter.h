#pragma once
#include <dxgi1_6.h>
#include <wrl.h>
#include <string>

namespace KSEngine
{
	class KSAdapter
	{
	public:
		KSAdapter();
		~KSAdapter();

		KSAdapter(const KSAdapter&) = delete;
		KSAdapter& operator=(const KSAdapter&) = delete;

		// Enumerate and select an adapter (returns true on success)
		bool Init(IDXGIFactory6* factory, bool highPerformance = true, bool showLogging = true);

		IDXGIAdapter1* Get() const
		{
			return m_adapter.Get();
		}
		std::wstring GetDescription() const;
		DXGI_ADAPTER_DESC1 GetDescStruct() const;

	private:
		Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
		DXGI_ADAPTER_DESC1 m_desc = {};
	};
}