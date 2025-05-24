#include "Camera.h"
#include "imgui.h"
#include "../KSContext.h"
using namespace DirectX;

Camera::Camera()
	: m_pos(0.0f, 0.0f, -2.0f), m_target(0.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f),
	m_fovY(XM_PIDIV4), m_aspect(16.0f / 9.0f), m_nearZ(0.1f), m_farZ(100.0f)
{
	XMStoreFloat4x4(&m_view, XMMatrixIdentity());
	XMStoreFloat4x4(&m_proj, XMMatrixIdentity());
}

void Camera::Init(float aspect, float fovY, float nearZ, float farZ)
{
	m_aspect = aspect;
	m_fovY = fovY;
	m_nearZ = nearZ;
	m_farZ = farZ;
	Update();
}

void Camera::SetLookAt(const XMFLOAT3& eye, const XMFLOAT3& at, const XMFLOAT3& up)
{
	m_pos = eye;
	m_target = at;
	m_up = up;
	XMVECTOR vEye = XMLoadFloat3(&eye);
	XMVECTOR vAt = XMLoadFloat3(&at);
	XMVECTOR vUp = XMLoadFloat3(&up);
	XMMATRIX V = XMMatrixLookAtLH(vEye, vAt, vUp);
	XMStoreFloat4x4(&m_view, V);
}

void Camera::Update()
{
	// Update projection
	XMMATRIX P = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
	XMStoreFloat4x4(&m_proj, P);
	// Update view
	SetLookAt(m_pos, m_target, m_up);
}

void Camera::DrawUI()
{
	ImGui::Begin("Camera Controls");
	bool changed = false;
	changed |= ImGui::InputFloat3("Position", &m_pos.x, "%.2f");
	changed |= ImGui::InputFloat3("Target", &m_target.x, "%.2f");
	changed |= ImGui::InputFloat3("Up", &m_up.x, "%.2f");
	changed |= ImGui::SliderAngle("FOV Y", &m_fovY, 10.0f, 120.0f);
	if (changed || ImGui::Button("Update Camera"))
		Update();
	ImGui::End();
}

void Camera::CreateConstantBuffer()
{
	auto& dx12 = KSContext::Instance().GetDirectX12();
	auto device = dx12.GetDevice();
	ID3D12CommandQueue* queue = dx12.GetCommandQueue();

	// Create a temporary command allocator and command list for resource upload
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList));

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
}

void Camera::UpdateConstantBuffer()
{
	CameraCB cbData;
	DirectX::XMMATRIX view = GetView();
	DirectX::XMMATRIX proj = GetProj();
	DirectX::XMStoreFloat4x4(&cbData.view, DirectX::XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&cbData.proj, DirectX::XMMatrixTranspose(proj));

	void* mapped = nullptr;
	m_cameraCB->Map(0, nullptr, &mapped);
	memcpy(mapped, &cbData, sizeof(cbData));
	m_cameraCB->Unmap(0, nullptr);
}

D3D12_GPU_VIRTUAL_ADDRESS Camera::GetCBVGPUAddress() const
{
	return m_cameraCB->GetGPUVirtualAddress();
}

XMMATRIX Camera::GetView() const
{
	return XMLoadFloat4x4(&m_view);
}
XMMATRIX Camera::GetProj() const
{
	return XMLoadFloat4x4(&m_proj);
}