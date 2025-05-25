#include "Model.h"


namespace KSEngine
{
	void Model::Draw(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_VIRTUAL_ADDRESS cameraCB) const
	{
		size_t count = std::min(meshes.size(), materials.size());
		for (size_t i = 0; i < count; ++i)
		{
			if (materials[i])
				materials[i]->Bind(cmdList);

			// Set the camera constant buffer after binding the material (root signature)
			cmdList->SetGraphicsRootConstantBufferView(0, cameraCB);

			if (meshes[i])
				meshes[i]->Draw(cmdList);
		}
	}
}
