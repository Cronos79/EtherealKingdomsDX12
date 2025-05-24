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

KSDirectX12::KSDirectX12()
{

}

KSDirectX12::~KSDirectX12()
{
	if (m_commandQueue && m_fence && m_fenceEvent)
	{
		m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
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
	LOG_INFO(L"Initializing DirectX 12...");
	bool result = false;
	// Setup debug layer
	SetupDebugLayer();
	result = CreateFactory();
	if (!result)
	{
		LOG_ERROR(L"Failed to create DirectX 12 factory.");
		return false;
	}
	result = CreateDevice();
	if (!result)
	{
		LOG_ERROR(L"Failed to create DirectX 12 device.");
		return false;
	}
	result = CreateCommandQueue();
	if (!result)
	{
		LOG_ERROR(L"Failed to create DirectX 12 command queue.");
		return false;
	}
	result = CreateSwapChain();
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
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	Present();
}

void KSDirectX12::Present()
{
	m_swapChain->Present(1, 0);

	// Signal and increment the fence value
	const UINT64 currentFenceValue = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), currentFenceValue);
	m_fenceValue++;

	// Update frame index
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

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

bool KSDirectX12::CreateFactory()
{
	// Create DirectX12 DXGIFactory2 with debug support
#if defined(_DEBUG)
	HRESULT hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to create DirectX 12 factory with debug support. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	LOG_INFO(L"DirectX 12 factory with debug support created successfully.");
#else
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to create DirectX 12 factory. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	LOG_INFO(L"DirectX 12 factory created successfully.");
#endif
	
	// Check for hardware support
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(m_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		for (UINT i = 0;
			factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			m_adapter->GetDesc1(&desc);
			LOG_INFO(L"Found hardware adapter: {}", desc.Description);
			m_adapter.Reset();
		}
	}
	else
	{
		LOG_WARNING(L"Failed to query IDXGIFactory6 for hardware adapters.");
		return false;
	}
	LOG_INFO(L"DirectX 12 hardware support check completed.");
	return true;	
}

bool KSDirectX12::CreateDevice()
{
	// Create DirectX 12 device
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(m_factory->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		if (SUCCEEDED(factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter))))
		{
			if (FAILED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device))))
			{
				LOG_ERROR(L"Failed to create DirectX 12 device.");
				return false;
			}
			LOG_INFO(L"DirectX 12 device created successfully.");
		}
		else
		{
			LOG_ERROR(L"Failed to enumerate DirectX 12 adapters.");
			return false;
		}
	}
	else
	{
		LOG_ERROR(L"Failed to query IDXGIFactory6 for DirectX 12 device.");
		return false;
	}
	return true;
}

bool KSDirectX12::CreateCommandQueue()
{
	// Create command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queueDesc.NodeMask = 0;
	HRESULT hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to create DirectX 12 command queue. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	LOG_INFO(L"DirectX 12 command queue created successfully.");
	return true;
}

bool KSDirectX12::CreateSwapChain()
{
	// Create swap chain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = s_BufferCount;
	swapChainDesc.Width = KSContext::Instance().GetWidth();
	swapChainDesc.Height = KSContext::Instance().GetHeight();
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	
	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		KSContext::Instance().GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1
	);
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to create DirectX 12 swap chain. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	hr = swapChain1.As(&m_swapChain); // m_swapChain is ComPtr<IDXGISwapChain3>
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to query IDXGISwapChain3 from swap chain. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	LOG_INFO(L"DirectX 12 swap chain created successfully.");
	return true;
}

bool KSDirectX12::CreateCommandAllocator()
{
	// Create command allocator
	HRESULT hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
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
	HRESULT hr = m_device->CreateCommandList(
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
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = s_BufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap))))
	{
		LOG_ERROR(L"Failed to create RTV descriptor heap.");
		return false;
	}

	m_renderTargets.resize(s_BufferCount);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < s_BufferCount; ++i)
	{
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]))))
		{
			LOG_ERROR(L"Failed to get swap chain buffer {}.", i);
			return false;
		}
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	LOG_INFO(L"DirectX 12 render targets created successfully.");
	return true;
}

bool KSDirectX12::CreateFence()
{
	if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
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
	HRESULT hr = m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	if (FAILED(hr))
	{
		LOG_ERROR(L"Failed to create SRV descriptor heap. HRESULT: {}", KSLogger::FormatHRESULT(hr));
		return false;
	}
	LOG_INFO(L"SRV descriptor heap created successfully.");
	return true;
}

