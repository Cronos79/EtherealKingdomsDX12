#include "AssetManager.h"
#include <fstream>
#include <nlohmann/json.hpp> // Use nlohmann/json for JSON parsing
#include "../Graphics/Material.h"
#include "Texture.h"
#include "Sound.h"
#include "Engine/Graphics/Model.h"

namespace KSEngine
{
	AssetManager& AssetManager::Instance()
	{
		static AssetManager instance;
		return instance;
	}

	AssetBundle* AssetManager::Load(const std::string& assetName)
	{
		auto it = m_bundles.find(assetName);
		if (it != m_bundles.end()) return it->second.get();

		auto bundle = LoadBundleFromManifest(assetName);
		if (!bundle) return nullptr;
		auto ptr = bundle.get();
		m_bundles[assetName] = std::move(bundle);
		return ptr;
	}

	AssetBundle* AssetManager::Get(const std::string& assetName) const
	{
		auto it = m_bundles.find(assetName);
		return it != m_bundles.end() ? it->second.get() : nullptr;
	}

	void AssetManager::Clear()
	{
		m_bundles.clear();
	}

	std::unique_ptr<AssetBundle> AssetManager::LoadBundleFromManifest(const std::string& assetName)
	{
		std::string manifestPath = "Assets/" + assetName + ".asset.json";
		std::ifstream file(manifestPath);
		if (!file.is_open()) return nullptr;

		nlohmann::json j;
		file >> j;

		auto bundle = std::make_unique<AssetBundle>();

		// Load model
		if (j.contains("model"))
		{
			// bundle->model = std::make_unique<Model>();
			// bundle->model->LoadFromFile(j["model"]);
			// (Replace with your actual model loading code)
		}

		// Load textures
		if (j.contains("textures"))
		{
			for (const auto& texPath : j["textures"])
			{
				// auto tex = std::make_unique<Texture>();
				// tex->LoadFromFile(texPath);
				// bundle->textures.push_back(std::move(tex));
				// (Replace with your actual texture loading code)
			}
		}

		// Load material
		if (j.contains("material"))
		{
			// bundle->material = std::make_unique<Material>();
			// bundle->material->LoadFromFile(j["material"]);
			// (Replace with your actual material loading code)
		}

		// Load sounds
		if (j.contains("sounds"))
		{
			for (const auto& sndPath : j["sounds"])
			{
				// auto snd = std::make_unique<Sound>();
				// snd->LoadFromFile(sndPath);
				// bundle->sounds.push_back(std::move(snd));
				// (Replace with your actual sound loading code)
			}
		}

		return bundle;
	}
}