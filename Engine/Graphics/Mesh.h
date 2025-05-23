#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>

class Mesh {
public:
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
	};

	Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	void Draw(ID3D12GraphicsCommandList* cmdList) const;

	UINT GetIndexCount() const
	{
		return m_indexCount;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferUpload;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferUpload;

	D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
	D3D12_INDEX_BUFFER_VIEW m_ibView = {};
	UINT m_indexCount = 0;
};