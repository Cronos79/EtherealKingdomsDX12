#include "SceneManager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include "AssetManager.h"
#include "GameObject.h"
#include "Scene.h"
#include "../KSContext.h"
#include "Engine/Graphics/Model.h"
#include "../Logger/KSLogger.h"

namespace KSEngine
{
	SceneManager& SceneManager::Instance()
	{
		static SceneManager instance;
		return instance;
	}

	bool SceneManager::LoadScene(const std::string& sceneName)
	{
		UnloadCurrentScene();

		std::string scenePath = "Assets/Scenes/" + sceneName + ".scene.json";
		std::ifstream file(scenePath);
		if (!file.is_open())
			return false;

		nlohmann::json j;
		file >> j;

		m_currentScene = std::make_unique<Scene>(sceneName);		
		LOG_INFO(L"Loading scene: {} from path: {}", std::wstring(sceneName.begin(), sceneName.end()), std::wstring(scenePath.begin(), scenePath.end()));

		if (j.contains("objects") && j["objects"].is_array())
		{
			for (const auto& obj : j["objects"])
			{
				std::string objName = obj.value("name", "Unnamed");
				std::string modelName = obj.value("model", "");
				auto position = obj.value("position", std::vector<float>{0, 0, 0});

				// Create GameObject
				auto gameObject = std::make_shared<GameObject>(objName);

				// Set position if available
				if (position.size() == 3)
				{
					gameObject->GetTransform().position = { position[0], position[1], position[2] };
				}

				// Load model (using AssetManager)
				auto modelPtr = AssetManager::Instance().GetModel(modelName);
				gameObject->SetModel(modelPtr);

				m_currentScene->AddObject(gameObject);
			}
		}

		return m_currentScene != nullptr;
	}

	void SceneManager::UnloadCurrentScene()
	{
		if (m_currentScene)
		{
			m_currentScene->Clear();
			m_currentScene.reset();
		}
	}

	Scene* SceneManager::GetCurrentScene()
	{
		return m_currentScene.get();
	}

	const Scene* SceneManager::GetCurrentScene() const
	{
		return m_currentScene.get();
	}

	void SceneManager::Update(float deltaTime)
	{
		if (m_currentScene)
			m_currentScene->Update(deltaTime);
	}

	void SceneManager::Render(Camera* camera)
	{
		if (m_currentScene)
			m_currentScene->Render(camera);
	}
}