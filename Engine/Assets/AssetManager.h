#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

// Forward declarations

namespace KSEngine
{
	class Model;
	class Texture;
	class Material;
	class Sound;

	struct AssetBundle {
		std::unique_ptr<KSEngine::Model> model;
		std::vector<std::unique_ptr<KSEngine::Texture>> textures;
		std::unique_ptr<KSEngine::Material> material;
		std::vector<std::unique_ptr<KSEngine::Sound>> sounds;
		// Add more as needed
	};

	class AssetManager {
	public:
		static AssetManager& Instance();

		// Loads all assets for a logical name (e.g., "Hero1")
		AssetBundle* Load(const std::string& assetName);

		// Optionally, get already loaded bundle
		AssetBundle* Get(const std::string& assetName) const;

		void Clear();

	private:
		AssetManager() = default;
		std::unordered_map<std::string, std::unique_ptr<AssetBundle>> m_bundles;

		// Internal helpers
		std::unique_ptr<AssetBundle> LoadBundleFromManifest(const std::string& assetName);
	};
}