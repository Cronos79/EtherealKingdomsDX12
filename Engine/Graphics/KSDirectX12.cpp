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
		m_fence.Signal(m_commandQueue.Get());
		m_fence.Wait();

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
		result = m_adapter.Init(m_factory.Get(), true, showDebugInfo);
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
		result = m_commandAllocator.Init(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 command allocator.");
			return false;
		}
		result = m_commandList.Init(m_device, m_commandAllocator, D3D12_COMMAND_LIST_TYPE_DIRECT, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 command list.");
			return false;
		}
		result = m_renderTargets.Init(m_device, m_swapChain, s_BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create DirectX 12 render targets.");
			return false;
		}
		result = m_fence.Init(m_device, 0, showDebugInfo);
		if (!result)
		{
			LOG_ERROR(L"Failed to create fence.");
			return false;
		}
		result = m_srvHeap.Init(m_device, 100, showDebugInfo);
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
		m_commandAllocator.Reset();
		m_commandList.Reset(m_commandAllocator, nullptr);

		// Set descriptor heaps (SRV heap for ImGui and any shaders)
		ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
		m_commandList.Get()->SetDescriptorHeaps(_countof(heaps), heaps);

		// Set render target
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_renderTargets.GetRenderTarget(m_frameIndex),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		m_commandList.Get()->ResourceBarrier(1, &barrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderTargets.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_renderTargets.GetRTVDescriptorSize());
		m_commandList.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		// Clear render target
		const float clearColor[] = { 0.1f, 0.1f, 0.2f, 1.0f };
		m_commandList.Get()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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
			m_renderTargets.GetRenderTarget(m_frameIndex),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		m_commandList.Get()->ResourceBarrier(1, &barrier);

		m_commandList.Get()->Close();

		// Execute command list
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_commandQueue.Get()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		Present();
	}

	void KSDirectX12::Present()
	{
		m_swapChain.Get()->Present(1, 0);

		m_fence.Signal(m_commandQueue.Get());
		m_fence.Wait();
		m_fence.IncrementValue();

		m_frameIndex = m_swapChain.GetCurrentBackBufferIndex();
	}

	void KSDirectX12::SetupDebugLayer()
	{
#if defined(_DEBUG)
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugController))))
		{
			m_debugController->EnableDebugLayer();
			if (showDebugInfo)
			{
				LOG_INFO(L"DirectX 12 debug layer enabled.");
			}
		}
		else
		{
			LOG_WARNING(L"Failed to enable DirectX 12 debug layer.");
		}
#endif
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
			m_srvHeap.GetCPUHandleStart(),
			m_srvHeap.GetGPUHandleStart()
		);
		return true;
	}
}

