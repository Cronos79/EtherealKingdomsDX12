#pragma once
#include <DirectXMath.h>

namespace KSEngine
{
	struct CameraCB
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 proj;
	};
}