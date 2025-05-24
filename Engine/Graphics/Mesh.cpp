#include "Mesh.h"
#include <d3dx12.h> // For CD3DX12 helpers

namespace KSEngine
{
	Mesh::Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,
		const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_indexCount = static_cast<UINT>(indices.size());

		// --- Vertex Buffer ---
		const UINT vbSize = UINT(vertices.size() * sizeof(Vertex));
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);

		device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(&m_vertexBuffer));

		// Upload heap for vertex buffer
		CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
		device->CreateCommittedResource(
			&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &vbDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m_vertexBufferUpload));

		// Copy vertex data
		void* mappedData = nullptr;
		m_vertexBufferUpload->Map(0, nullptr, &mappedData);
		memcpy(mappedData, vertices.data(), vbSize);
		m_vertexBufferUpload->Unmap(0, nullptr);

		cmdList->CopyBufferRegion(m_vertexBuffer.Get(), 0, m_vertexBufferUpload.Get(), 0, vbSize);

		// Transition to VERTEX_AND_CONSTANT_BUFFER
		CD3DX12_RESOURCE_BARRIER vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		cmdList->ResourceBarrier(1, &vbBarrier);

		m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vbView.SizeInBytes = vbSize;
		m_vbView.StrideInBytes = sizeof(Vertex);

		// --- Index Buffer ---
		const UINT ibSize = UINT(indices.size() * sizeof(uint32_t));
		CD3DX12_RESOURCE_DESC ibDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);

		device->CreateCommittedResource(
			&heapProps, D3D12_HEAP_FLAG_NONE, &ibDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
			IID_PPV_ARGS(&m_indexBuffer));

		device->CreateCommittedResource(
			&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &ibDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(&m_indexBufferUpload));

		m_indexBufferUpload->Map(0, nullptr, &mappedData);
		memcpy(mappedData, indices.data(), ibSize);
		m_indexBufferUpload->Unmap(0, nullptr);

		cmdList->CopyBufferRegion(m_indexBuffer.Get(), 0, m_indexBufferUpload.Get(), 0, ibSize);

		// Transition to INDEX_BUFFER
		CD3DX12_RESOURCE_BARRIER ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		cmdList->ResourceBarrier(1, &ibBarrier);

		m_ibView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_ibView.SizeInBytes = ibSize;
		m_ibView.Format = DXGI_FORMAT_R32_UINT;
	}

	void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) const
	{
		cmdList->IASetVertexBuffers(0, 1, &m_vbView);
		cmdList->IASetIndexBuffer(&m_ibView);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
	}
}