#pragma once
#include <DirectXMath.h>

struct CameraCB
{
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
};