#include "GameConfig.h"
#include <fstream>
#include <nlohmann/json.hpp> // Or your preferred JSON lib

namespace KSEngine
{
	bool GameConfig::LoadFromFile(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file.is_open()) return false;
		nlohmann::json j;
		file >> j;
		m_initialScene = j.value("initialScene", "DefaultScene");
		// Load other options as needed
		return true;
	}

	std::string GameConfig::GetInitialScene() const
	{
		return m_initialScene;
	}

	bool GameConfig::LoadMaterialsFromFile(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file.is_open()) return false;
		nlohmann::json j;
		file >> j;
		for (const auto& item : j.items())
		{
			const std::string& name = item.key();
			const auto& materialData = item.value();
			MaterialData data;
			data.name = name;
			data.VSPath = materialData.value("VertexShader", "");
			data.PSPath = materialData.value("PixelShader", "");
			m_materials[name] = data;
		}
		return true;
	}

	const KSEngine::MaterialData* GameConfig::GetMaterialData(const std::string& name) const
	{
		auto it = m_materials.find(name);
		if (it != m_materials.end())
			return &it->second;
		return nullptr;
	}

}