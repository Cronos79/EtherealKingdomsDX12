#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxgi.h>

namespace KSEngine
{
	class KSDevice
	{
	public:
		KSDevice();
		~KSDevice();

		KSDevice(const KSDevice&) = delete;
		KSDevice& operator=(const KSDevice&) = delete;

		// Initializes the device using the given adapter (can be nullptr for default)
		bool Init(IDXGIAdapter1* adapter = nullptr, bool showLogging = true);

		ID3D12Device14* Get() const
		{
			return m_device.Get();
		}

	private:
		Microsoft::WRL::ComPtr<ID3D12Device14> m_device;
	};
}