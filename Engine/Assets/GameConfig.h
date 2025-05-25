#pragma once
#include <string>
#include <unordered_map>

namespace KSEngine
{
	struct MaterialData {
		std::string name;
		std::string VSPath;
		std::string PSPath;
		// Add more material properties as needed
	};
	class GameConfig {
	public:
		bool LoadFromFile(const std::string& filename);
		std::string GetInitialScene() const;
		bool LoadMaterialsFromFile(const std::string& filename);
		// Get MaterialData by name
		const MaterialData* GetMaterialData(const std::string& name) const;
		

	private:
		std::string m_initialScene;
		std::unordered_map<std::string, MaterialData> m_materials;
		// Add more config fields as needed
	};
}