#include "Model.h"
#include <algorithm>
#include "../KSContext.h"

bool Model::LoadFromFile(ID3D12Device* device, const std::string& filename)
{
	// For now, just create a simple triangle as a placeholder.
 // In a real loader, you would parse 'filename' and fill m_meshes/m_materials accordingly.

 // Example mesh data (triangle)
	std::vector<Mesh::Vertex> vertices = {
		{ {  0.0f,  0.5f, 0.0f }, {0,0,1}, {0.5f, 0.0f} },
		{ {  0.5f, -0.5f, 0.0f }, {0,0,1}, {1.0f, 1.0f} },
		{ { -0.5f, -0.5f, 0.0f }, {0,0,1}, {0.0f, 1.0f} },
	};
	std::vector<uint32_t> indices = { 0, 1, 2 };

	// Create a temporary command allocator and command list for upload
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList));

	// Create mesh using the upload command list
	m_meshes.push_back(std::make_unique<Mesh>(device, uploadCmdList.Get(), vertices, indices));

	// Close and execute the upload command list
	uploadCmdList->Close();
	ID3D12CommandList* lists[] = { uploadCmdList.Get() };
	auto queue = KSContext::Instance().GetDirectX12().GetCommandQueue();
	queue->ExecuteCommandLists(1, lists);

	// Wait for GPU to finish uploading
	auto fence = KSContext::Instance().GetDirectX12().GetFence();
	UINT64 fenceValue = KSContext::Instance().GetDirectX12().GetFenceValue();
	queue->Signal(fence, fenceValue);
	KSContext::Instance().GetDirectX12().IncrementFenceValue();
	HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fence->GetCompletedValue() < fenceValue)
	{
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	CloseHandle(fenceEvent);

	m_materials.push_back(std::make_unique<Material>(
		device,
		L"C:/Projects/KingdomSim/x64/Debug/Shaders/VS.cso",
		L"C:/Projects/KingdomSim/x64/Debug/Shaders/PS.cso"
	));

	return true;
}

void Model::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_VIRTUAL_ADDRESS cameraCBAddress)
{
	size_t count = std::min(m_meshes.size(), m_materials.size());
	for (size_t i = 0; i < count; ++i)
	{
		if (m_materials[i])
			m_materials[i]->Bind(cmdList);

		// Set the camera constant buffer after binding the material (root signature)
		cmdList->SetGraphicsRootConstantBufferView(0, cameraCBAddress);

		if (m_meshes[i])
			m_meshes[i]->Draw(cmdList);
	}
}
