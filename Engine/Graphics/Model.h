#pragma once
#include <vector>
#include <memory>
#include <string>
#include <d3d12.h>
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"

namespace KSEngine
{
	class Model {
	public:
		void Draw(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_VIRTUAL_ADDRESS cameraCB) const;

		std::vector<std::unique_ptr<Mesh>> meshes;
		std::vector<std::unique_ptr<Material>> materials;
		std::vector<std::unique_ptr<Texture>> textures;
		std::string name;		
	};
}