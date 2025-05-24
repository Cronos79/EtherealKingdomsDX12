#include "KSDirectX12.h"
#include "../Logger/KSLogger.h"

#if defined(_DEBUG)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif
#include "../KSContext.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

using namespace Microsoft::WRL;

namespace KSEngine
{
	KSDirectX12::KSDirectX12()
	{

	}

	KSDirectX12::~KSDirectX12()
	{
		if (m_commandQueue.Get() && m_fence && m_fenceEvent)
		{
			m_commandQueue.Get()->Signal(m_fence.Get(), m_fenceValue);
			m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
			CloseHandle(m_fenceEvent);
		}

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	bool KSDirectX12::Initialize()
	{
		bool enableDebug = false;
#if defined(_DEBUG)
		enableDebug = true;
		// Setup debug layer
		SetupDebugLayer();
#endif
		if (showDebugInfo)
		{
			LOG_INFO(L"Initializing DirectX 12...");
		}		
		bool result = m_factory.Init(enableDebug, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 factory.");
			return false;
		}
		result = CreateAdapter();
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 adapter.");
			return false;
		}
		result = m_device.Init(m_adapter.Get(), showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 device.");
			return false;
		}
		result = m_commandQueue.Init(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 command queue.");
			return false;
		}
		result = m_swapChain.Init(m_factory, m_commandQueue, KSContext::Instance().GetHWND(), KSContext::Instance().GetWidth(), KSContext::Instance().GetHeight(), s_BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 swap chain.");
			return false;
		}
		result = CreateCommandAllocator();
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 command allocator.");
			return false;
		}
		result = CreateCommandList();
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 command list.");
			return false;
		}
		result = CreateRenderTargets();
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 render targets.");
			return false;
		}
		result = CreateFence();
		if (!result)
		{
			LOG_ERROR(L"Failed to create fence.");
			return false;
		}
		result = CreateSRVHeap();
		if (!result)
		{
			LOG_ERROR(L"Failed to create SRV heap.");
			return false;
		}
		// Setup ImGui
		SetupImGui();

		LOG_INFO(L"DirectX 12 initialized successfully.");
		return result;
	}

	void KSDirectX12::BeginFrame()
	{
		// Reset allocator and command list
		m_commandAllocator->Reset();
		m_commandList->Reset(m_commandAllocator.Get(), nullptr);

		// Set descriptor heaps (SRV heap for ImGui and any shaders)
		ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

		// Set render target
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		m_commandList->ResourceBarrier(1, &barrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Clear render target
		const float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
		m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void KSDirectX12::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
		// Transition back to present
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets[m_frameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		m_commandList->ResourceBarrier(1, &barrier);

		m_commandList->Close();

		// Execute command list
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue.Get()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		Present();
	}

	void KSDirectX12::Present()
	{
		m_swapChain.Get()->Present(1, 0);

		// Signal and increment the fence value
		const UINT64 currentFenceValue = m_fenceValue;
		m_commandQueue.Get()->Signal(m_fence.Get(), currentFenceValue);
		m_fenceValue++;

		// Update frame index
		m_frameIndex = m_swapChain.GetCurrentBackBufferIndex();

		// Wait until the previous frame is finished
		if (m_fence->GetCompletedValue() < currentFenceValue)
		{
			m_fence->SetEventOnCompletion(currentFenceValue, m_fenceEvent);
			WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void KSDirectX12::SetupDebugLayer()
	{
#if defined(_DEBUG)
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
		{
			m_debugController->EnableDebugLayer();
			LOG_INFO(L"DirectX 12 debug layer enabled.");
		}
		else
		{
			LOG_WARNING(L"Failed to enable DirectX 12 debug layer.");
		}
#endif
	}	

	bool KSDirectX12::CreateAdapter()
	{
		// Create DirectX 12 device
		ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(m_factory.Get()->QueryInterface(IID_PPV_ARGS(&factory6))))
		{
			if (SUCCEEDED(factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter))))
			{
				if (showDebugInfo)
				{
					LOG_INFO(L"DirectX 12 adapter created successfully.");
				}
				return true;
			}
		}
		return false;
	}	

	bool KSDirectX12::CreateCommandAllocator()
	{
		// Create command allocator
		HRESULT hr = m_device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create DirectX 12 command allocator. HRESULT: {}", KSLogger::FormatHRESULT(hr));
			return false;
		}
		LOG_INFO(L"DirectX 12 command allocator created successfully.");
		return true;
	}

	bool KSDirectX12::CreateCommandList()
	{
		// Create command list
		HRESULT hr = m_device.Get()->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_commandAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(&m_commandList)
		);
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create DirectX 12 command list. HRESULT: {}", KSLogger::FormatHRESULT(hr));
			return false;
		}
		hr = m_commandList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to close DirectX 12 command list. HRESULT: {}", KSLogger::FormatHRESULT(hr));
			return false;
		}
		LOG_INFO(L"DirectX 12 command list created successfully.");
		return true;
	}

	bool KSDirectX12::CreateRenderTargets()
	{
		m_rtvDescriptorSize = m_device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = s_BufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(m_device.Get()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
		{
			LOG_ERROR(L"Failed to create RTV descriptor heap.");
			return false;
		}

		m_renderTargets.resize(s_BufferCount);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < s_BufferCount; ++i)
		{
			if (FAILED(m_swapChain.Get()->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
			{
				LOG_ERROR(L"Failed to get swap chain buffer {}.", i);
				return false;
			}
			m_device.Get()->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

		m_frameIndex = m_swapChain.GetCurrentBackBufferIndex();
		LOG_INFO(L"DirectX 12 render targets created successfully.");
		return true;
	}

	bool KSDirectX12::CreateFence()
	{
		if (FAILED(m_device.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
		{
			LOG_ERROR(L"Failed to create fence.");
			return false;
		}
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (!m_fenceEvent)
		{
			LOG_ERROR(L"Failed to create fence event.");
			return false;
		}
		LOG_INFO(L"DirectX 12 fence created successfully.");
		return true;
	}

	bool KSDirectX12::SetupImGui()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(KSContext::Instance().GetHWND()); // Pass your HWND
		ImGui_ImplDX12_Init(
			m_device.Get(),
			s_BufferCount,
			DXGI_FORMAT_R8G8B8A8_UNORM, // Use your swapchain format
			m_srvHeap.Get(), // SRV heap, not RTV heap
			m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
			m_srvHeap->GetGPUDescriptorHandleForHeapStart()
		);
		return true;
	}

	bool KSDirectX12::CreateSRVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 100; // Adjust as needed
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = 0;
		HRESULT hr = m_device.Get()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
		if (FAILED(hr))
		{
			LOG_ERROR(L"Failed to create SRV descriptor heap. HRESULT: {}", KSLogger::FormatHRESULT(hr));
			return false;
		}
		LOG_INFO(L"SRV descriptor heap created successfully.");
		return true;
	}
}

