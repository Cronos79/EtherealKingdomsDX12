#pragma once
#include <string>
#include <unordered_map>
#include <memory>

struct aiScene;

namespace KSEngine
{
	class Model;

	struct ModelManifestEntry {
		std::string fbxPath;
		std::string materialName;
	};

	class AssetManager {
	public:
		static AssetManager& Instance();

		std::shared_ptr<Model> GetModel(const std::string& name);
		void LoadManifest(const std::string& manifestPath);
		void Clear();

	private:
		// Fix: Add reference for Model parameter
		void PopulateModelFromAssimp(const aiScene* scene, const std::string& basePath, Model& model);

		AssetManager() = default;

		std::unordered_map<std::string, ModelManifestEntry> m_modelManifest;
		std::unordered_map<std::string, std::shared_ptr<Model>> m_modelCache;
	};
}