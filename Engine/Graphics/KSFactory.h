#pragma once
#include <dxgi1_6.h>
#include <wrl.h>

namespace KSEngine
{
	class KSFactory
	{
	public:
		KSFactory();
		~KSFactory();

		KSFactory(const KSFactory&) = delete;
		KSFactory& operator=(const KSFactory&) = delete;

		// Initializes the factory. Pass true to enable debug layer.
		bool Init(bool enableDebug, bool showDebug = true);

		IDXGIFactory7* Get() const
		{
			return m_factory.Get();
		}

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory7> m_factory;
	};
}