#pragma once
#include <stdint.h>
#include <memory>
#include <wrl/client.h>
#include "Engine/Graphics/Camera.h"
#include <packages/Microsoft.Direct3D.D3D12.1.615.1/build/native/include/d3d12.h>
#include "Engine/Graphics/Model.h"

// Forward declarations
class Mesh;
class Material;

class GameApp {
public:
	GameApp();
	~GameApp();

	void Initialize();
	int32_t Run();

	void AddModel(const std::string& filename);

private:
	void RenderFrame();

private:
	KSEngine::Camera m_camera;
	std::vector<std::unique_ptr<KSEngine::Model>> m_models;
};