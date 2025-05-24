#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include <d3d12.h>
#include "CameraCB.h"

class Camera
{
public:
	Camera();

	// Setup
	void Init(float aspect, float fovY = DirectX::XM_PIDIV4, float nearZ = 0.1f, float farZ = 100.0f);
	// ImGui UI
	void DrawUI();
	// Update view/proj matrices if needed
	void Update();
	void HandleInput();

	void CreateConstantBuffer();
	void UpdateConstantBuffer();
	D3D12_GPU_VIRTUAL_ADDRESS GetCBVGPUAddress() const;

	// Getters (as XMMATRIX for GPU upload)
	DirectX::XMMATRIX GetView() const;
	DirectX::XMMATRIX GetProj() const;

	// Getters (as XMFLOAT4X4 for CPU-side copy)
	const DirectX::XMFLOAT4X4& GetViewMatrix() const
	{
		return m_view;
	}
	const DirectX::XMFLOAT4X4& GetProjMatrix() const
	{
		return m_proj;
	}

	// Camera state
	const DirectX::XMFLOAT3& GetPosition() const
	{
		return m_pos;
	}
	const DirectX::XMFLOAT3& GetTarget() const
	{
		return m_target;
	}
	const DirectX::XMFLOAT3& GetUp() const
	{
		return m_up;
	}

	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& at, const DirectX::XMFLOAT3& up);

private:
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_proj;
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_target;
	DirectX::XMFLOAT3 m_up;
	float m_fovY, m_aspect, m_nearZ, m_farZ;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_cameraCB;
	float m_zoomSpeed = 0.1f;
};