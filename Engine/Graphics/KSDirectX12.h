#pragma once
#include "Engine/Platform/WinInclude.h"

// Include DirectX 12 headers
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <wrl.h>

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
		return m_rtvHeap.Get();
	}
	ID3D12Resource* GetRenderTarget()
	{
		return m_renderTargets[m_frameIndex].Get();
	}
	ID3D12Fence* GetFence()
	{
		return m_fence.Get();
	}
	UINT64 GetFenceValue()
	{
		return m_fenceValue;
	}
	ID3D12DescriptorHeap* GetSRVHeap()
	{
		return m_srvHeap.Get();
	}
	void IncrementFenceValue()
	{
		m_fenceValue++;
	}

private:
	void Present();

	void SetupDebugLayer();
	bool CreateFactory();
	bool CreateDevice();
	bool CreateCommandQueue();
	bool CreateSwapChain();
	bool CreateCommandAllocator();
	bool CreateCommandList();
	bool CreateRenderTargets();
	bool CreateFence();
	bool SetupImGui();
	bool CreateSRVHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12Debug> m_debugController;
	Microsoft::WRL::ComPtr<IDXGIFactory7> m_factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_adapter;
	Microsoft::WRL::ComPtr<ID3D12Device14> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;

	const static UINT s_BufferCount = 2;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_renderTargets;
	UINT m_rtvDescriptorSize = 0;
	UINT m_frameIndex = 0;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;
};
