#include "GameApp.h"
#include "Engine/KSContext.h"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Material.h"

GameApp::GameApp()
{
}

GameApp::~GameApp()
{
}

void GameApp::Initialize()
{
	// Initialize the window and DirectX 12
	KSContext::Instance().Initialize(L"Kingdom game", 1920, 1080);

	// Create a simple triangle mesh
	std::vector<Mesh::Vertex> vertices = {
		{ {  0.0f,  0.5f, 0.0f }, {0,0,1}, {0.5f, 0.0f} },
		{ {  0.5f, -0.5f, 0.0f }, {0,0,1}, {1.0f, 1.0f} },
		{ { -0.5f, -0.5f, 0.0f }, {0,0,1}, {0.0f, 1.0f} },
	};
	std::vector<uint32_t> indices = { 0, 1, 2 };

	auto& dx12 = KSContext::Instance().GetDirectX12();
	auto device = dx12.GetDevice();
	ID3D12CommandQueue* queue = dx12.GetCommandQueue();

	// Create a temporary command allocator and command list for resource upload
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList));
	
	// Setup camera
	float aspect = 1920.0f / 1080.0f;
	m_camera.SetPerspective(DirectX::XM_PIDIV4, aspect, 0.1f, 100.0f);
	m_camera.SetLookAt({ 0,0,-2 }, { 0,0,0 }, { 0,1,0 });

	// Create camera constant buffer
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width = (sizeof(CameraCB) + 255) & ~255; // 256-byte aligned
	resDesc.Height = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_cameraCB)
	);

	// Close and execute the upload command list
	uploadCmdList->Close();
	ID3D12CommandList* lists[] = { uploadCmdList.Get() };
	queue->ExecuteCommandLists(1, lists);

	// Wait for GPU to finish uploading
	auto fence = dx12.GetFence();
	UINT64 fenceValue = dx12.GetFenceValue();
	queue->Signal(fence, fenceValue);
	dx12.IncrementFenceValue();
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fence->GetCompletedValue() < fenceValue)
	{
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	CloseHandle(fenceEvent);

	AddModel("TestModel.fbx");
}

int32_t GameApp::Run()
{
	while (KSContext::Instance().GetIsRunning())
	{
		// Process window messages
		if (auto msg = KSWindow::ProcessMessages())
		{
			KSContext::Instance().StopRunning();
			return *msg;
		}
		RenderFrame();
	}
}

void GameApp::AddModel(const std::string& filename)
{
	auto& dx12 = KSContext::Instance().GetDirectX12();
	auto device = dx12.GetDevice();
	m_models.push_back(std::make_unique<Model>());
	m_models[0]->LoadFromFile(device, filename);
}

void GameApp::SetCamera()
{
	// Update camera constant buffer
	CameraCB cbData;
	DirectX::XMMATRIX view = m_camera.GetView();
	DirectX::XMMATRIX proj = m_camera.GetProj();
	DirectX::XMStoreFloat4x4(&cbData.view, DirectX::XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&cbData.proj, DirectX::XMMatrixTranspose(proj));

	void* mapped = nullptr;
	m_cameraCB->Map(0, nullptr, &mapped);
	memcpy(mapped, &cbData, sizeof(cbData));
	m_cameraCB->Unmap(0, nullptr);
}

void GameApp::RenderFrame()
{
	auto& dx12 = KSContext::Instance().GetDirectX12();
	auto cmdList = dx12.GetCommandList();

	dx12.BeginFrame();

	SetCamera();

	// Set viewport, scissor, and topology
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)KSContext::Instance().GetWidth();
	viewport.Height = (float)KSContext::Instance().GetHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = KSContext::Instance().GetWidth();
	scissorRect.bottom = KSContext::Instance().GetHeight();

	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw the model
	for (auto& model : m_models)
	{
		if (model)
			model->Draw(cmdList, m_cameraCB->GetGPUVirtualAddress());
	}

	dx12.EndFrame();
}
