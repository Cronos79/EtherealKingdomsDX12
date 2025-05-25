#include "AssetManager.h"
#include "Engine/Graphics/Model.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Logger/KSLogger.h"
#include "../Graphics/Mesh.h"
#include "../KSContext.h"
#include "../Graphics/KSDirectX12.h"
#include <packages/Microsoft.Direct3D.D3D12.1.615.1/build/native/include/d3d12.h>

using json = nlohmann::json;

namespace KSEngine
{
	AssetManager& AssetManager::Instance()
	{
		static AssetManager instance;
		return instance;
	}

	void AssetManager::LoadManifest(const std::string& manifestPath)
	{
		std::ifstream file(manifestPath);
		if (!file.is_open())
		{
			LOG_ERROR(L"Failed to open model manifest file: {}", std::wstring(manifestPath.begin(), manifestPath.end()));
			return;
		}
		json j;
		file >> j;
		for (auto& [name, entry] : j.items())
		{
			ModelManifestEntry manifestEntry;
			manifestEntry.fbxPath = entry["FBXFilePath"];
			manifestEntry.materialName = entry.value("Material", ""); // fallback if not present
			m_modelManifest[name] = manifestEntry;
		}
	}

	std::shared_ptr<Model> AssetManager::GetModel(const std::string& name)
	{
		auto it = m_modelCache.find(name);
		if (it != m_modelCache.end())
			return it->second;

		// Check if ModelManifest is empty
		if (m_modelManifest.empty())
		{
			LOG_WARNING(L"Model manifest is empty. Please load the manifest first.");
			LoadManifest("Assets/Models/ModelManifest.json");
			if (!m_modelManifest.empty())
			{
				LOG_INFO(L"Loaded Model manifest.");
			}
			else
			{
				LOG_ERROR(L"Failed to load Model manifest.");
				return nullptr;
			}
		}

		auto manifestIt = m_modelManifest.find(name);
		if (manifestIt == m_modelManifest.end())
			return nullptr;

		const std::string& fbxPath = manifestIt->second.fbxPath;

		// Load with Assimp
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(fbxPath, aiProcess_Triangulate | aiProcess_FlipUVs);
		if (!scene || !scene->mRootNode)
			return nullptr;

		// Create and populate Model
		auto model = std::make_shared<Model>();
		model->name = name;
		// Log the model its loading with path
		LOG_INFO(L"Loading model: {} from path: {}", std::wstring(name.begin(), name.end()), std::wstring(fbxPath.begin(), fbxPath.end()));
		PopulateModelFromAssimp(scene, fbxPath, *model);

		m_modelCache[name] = model;
		return model;
	}

	// Helper function to fill Model with data from Assimp
	void AssetManager::PopulateModelFromAssimp(const aiScene* scene, const std::string& basePath, Model& model)
	{
		if (!scene->HasMeshes())
			return;

		// Get device and command list from your context (adjust as needed for your engine)
		auto& dx12 = KSContext::Instance().GetDirectX12();
		auto device = dx12.GetDevice();
		ID3D12CommandQueue* queue = dx12.GetCommandQueue();
		// Create a temporary command allocator and command list for upload
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCmdList;
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
		device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCmdList));

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh* ai_mesh = scene->mMeshes[i];
			std::vector<Mesh::Vertex> vertices;
			std::vector<uint32_t> indices;

			// Vertices
			for (unsigned int v = 0; v < ai_mesh->mNumVertices; ++v)
			{
				Mesh::Vertex vert{};
				vert.position = {
					ai_mesh->mVertices[v].x,
					ai_mesh->mVertices[v].y,
					ai_mesh->mVertices[v].z
				};
				vert.normal = ai_mesh->HasNormals() ?
					DirectX::XMFLOAT3(
						ai_mesh->mNormals[v].x,
						ai_mesh->mNormals[v].y,
						ai_mesh->mNormals[v].z
					) : DirectX::XMFLOAT3(0, 0, 0);

				if (ai_mesh->HasTextureCoords(0))
				{
					vert.texcoord = DirectX::XMFLOAT2(
						ai_mesh->mTextureCoords[0][v].x,
						ai_mesh->mTextureCoords[0][v].y
					);
				}
				else
				{
					vert.texcoord = DirectX::XMFLOAT2(0, 0);
				}
				vertices.push_back(vert);
			}

			// Indices
			for (unsigned int f = 0; f < ai_mesh->mNumFaces; ++f)
			{
				const aiFace& face = ai_mesh->mFaces[f];
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(static_cast<uint32_t>(face.mIndices[j]));
				}
			}

			// Create Mesh and add to model
			auto mesh = std::make_unique<Mesh>(device, uploadCmdList.Get(), vertices, indices);
			model.meshes.push_back(std::move(mesh));
		}

		model.materials.clear();
		// Load materials JSON once (ideally, do this at app startup)
		auto& gameConfig = KSContext::Instance().GetGameConfig();
		auto manifestIt = m_modelManifest.find(model.name);
		if (manifestIt == m_modelManifest.end())
		{
			LOG_ERROR(L"Model manifest entry not found for model: {}", std::wstring(model.name.begin(), model.name.end()));
			return;
		}

		 std::string& materialName = manifestIt->second.materialName;
		if (materialName.empty())
		{
			materialName = "YellowMaterial"; // Fallback to default material
		}

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			auto materialIt = gameConfig.GetMaterialData(materialName);
			if (!materialIt)
			{
				// Handle missing material (fallback or error)
				continue;
			}

			// Convert std::string to std::wstring
			std::wstring wvsPath(materialIt->VSPath.begin(), materialIt->VSPath.end());
			std::wstring wpsPath(materialIt->PSPath.begin(), materialIt->PSPath.end());

			auto material = std::make_unique<Material>(device, wvsPath, wpsPath);
			model.materials.push_back(std::move(material));
		}

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

	void AssetManager::Clear()
	{
		m_modelCache.clear();
	}
}