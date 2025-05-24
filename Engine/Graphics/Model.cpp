#include "Model.h"
#include <algorithm>
#include "../KSContext.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <functional>

namespace KSEngine
{
	bool Model::LoadFromFile(ID3D12Device* device, const std::string& filename)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			filename,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			// Optionally log error
			return false;
		}

		// Helper lambda to process a mesh
		auto processMesh = [&](aiMesh* mesh) -> std::unique_ptr<Mesh> {
			std::vector<Mesh::Vertex> vertices;
			std::vector<uint32_t> indices;

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
			{
				Mesh::Vertex vertex;
				vertex.position = {
					mesh->mVertices[i].x,
					mesh->mVertices[i].y,
					mesh->mVertices[i].z
				};
				vertex.normal = mesh->HasNormals() ?
					DirectX::XMFLOAT3(
						mesh->mNormals[i].x,
						mesh->mNormals[i].y,
						mesh->mNormals[i].z
					) : DirectX::XMFLOAT3(0, 0, 1);
				if (mesh->mTextureCoords[0])
				{
					vertex.texcoord = {
						mesh->mTextureCoords[0][i].x,
						mesh->mTextureCoords[0][i].y
					};
				}
				else
				{
					vertex.texcoord = { 0.0f, 0.0f };
				}
				vertices.push_back(vertex);
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
			{
				const aiFace& face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			// Create mesh using the upload command list
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList));

			auto meshPtr = std::make_unique<Mesh>(device, uploadCmdList.Get(), vertices, indices);

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

			return meshPtr;
			};

		// Recursively process nodes
		std::function<void(aiNode*)> processNode;
		processNode = [&](aiNode* node) {
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				m_meshes.push_back(processMesh(mesh));
			}
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
			{
				processNode(node->mChildren[i]);
			}
			};
		processNode(scene->mRootNode);

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
}