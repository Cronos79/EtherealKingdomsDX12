#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();

	// Set up a perspective projection
	void SetPerspective(float fovY, float aspect, float nearZ, float farZ);

	// Set up a look-at view matrix
	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& at, const DirectX::XMFLOAT3& up);

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

private:
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_proj;
};