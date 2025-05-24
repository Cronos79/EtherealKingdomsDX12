#pragma once
#include <vector>
#include <memory>
#include "Mesh.h"
#include "Material.h"

namespace KSEngine
{
	class Model {
	public:
		bool LoadFromFile(ID3D12Device* device, const std::string& filename);
		void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_VIRTUAL_ADDRESS cameraCBAddress);

	private:
		std::vector<std::unique_ptr<Mesh>> m_meshes;
		std::vector<std::unique_ptr<Material>> m_materials;
	};
}