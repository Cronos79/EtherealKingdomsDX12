#include "Camera.h"
using namespace DirectX;

Camera::Camera()
{
	// Identity by default
	XMStoreFloat4x4(&m_view, XMMatrixIdentity());
	XMStoreFloat4x4(&m_proj, XMMatrixIdentity());
}

void Camera::SetPerspective(float fovY, float aspect, float nearZ, float farZ)
{
	XMMATRIX P = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	XMStoreFloat4x4(&m_proj, P);
}

void Camera::SetLookAt(const XMFLOAT3& eye, const XMFLOAT3& at, const XMFLOAT3& up)
{
	XMVECTOR vEye = XMLoadFloat3(&eye);
	XMVECTOR vAt = XMLoadFloat3(&at);
	XMVECTOR vUp = XMLoadFloat3(&up);
	XMMATRIX V = XMMatrixLookAtLH(vEye, vAt, vUp);
	XMStoreFloat4x4(&m_view, V);
}

XMMATRIX Camera::GetView() const
{
	return XMLoadFloat4x4(&m_view);
}

XMMATRIX Camera::GetProj() const
{
	return XMLoadFloat4x4(&m_proj);
}
