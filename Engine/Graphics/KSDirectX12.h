#pragma once
#include "Engine/Platform/WinInclude.h"

// Include DirectX 12 headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <wrl.h>
#include "KSFactory.h"
#include "KSDevice.h"
#include "KSCommandQueue.h"
#include "KSSwapChain.h"
#include "KSCommandAllocator.h"
#include "KSCommandList.h"
#include "KSRenderTargets.h"
#include "KSFence.h"
#include "KSSRVHeap.h"
#include "KSAdapter.h"

namespace KSEngine
{
	class KSDirectX12
	{
	public:
		KSDirectX12();
		~KSDirectX12();
		KSDirectX12(const KSDirectX12&) = delete;
		KSDirectX12& operator=(const KSDirectX12&) = delete;

		bool Initialize();
		void BeginFrame();
		void EndFrame();

		// Getters
		ID3D12Device14* GetDevice()
		{
			return m_device.Get();
		}
		ID3D12CommandQueue* GetCommandQueue()
		{
			return m_commandQueue.Get();
		}
		IDXGISwapChain3* GetSwapChain()
		{
			return m_swapChain.Get();
		}
		ID3D12CommandAllocator* GetCommandAllocator()
		{
			return m_commandAllocator.Get();
		}
		ID3D12GraphicsCommandList* GetCommandList()
		{
			return m_commandList.Get();
		}
		ID3D12DescriptorHeap* GetRTVHeap()
		{
			return m_renderTargets.GetRTVHeap();
		}
		ID3D12Resource* GetRenderTarget()
		{
			return m_renderTargets.GetRenderTarget(m_frameIndex);
		}
		ID3D12Fence* GetFence()
		{
			return m_fence.Get();
		}
		UINT64 GetFenceValue()
		{
			return m_fence.GetValue();
		}
		ID3D12DescriptorHeap* GetSRVHeap()
		{
			return m_srvHeap.Get();
		}
		void IncrementFenceValue()
		{
			m_fence.IncrementValue();
		}

	private:
		void Present();
		void SetupDebugLayer();
		bool SetupImGui();

	private:
		bool showDebugInfo = false;
		const static UINT s_BufferCount = 2;
		UINT m_frameIndex = 0;

		Microsoft::WRL::ComPtr<ID3D12Debug> m_debugController;
		KSFactory m_factory;
		KSAdapter m_adapter;
		KSDevice m_device;
		KSCommandQueue m_commandQueue;
		KSSwapChain m_swapChain;
		KSCommandAllocator m_commandAllocator;
		KSCommandList m_commandList;
		KSRenderTargets m_renderTargets;
		KSFence m_fence;
		KSSRVHeap m_srvHeap;		
	};
}