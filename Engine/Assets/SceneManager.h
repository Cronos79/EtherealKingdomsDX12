#pragma once
#include <memory>
#include <string>
#include "Scene.h"
#include "../Graphics/Camera.h"

namespace KSEngine
{
	class SceneManager
	{
	public:
		static SceneManager& Instance();

		// Loads a new scene (unloads current if any)
		bool LoadScene(const std::string& sceneName);

		// Unloads the current scene
		void UnloadCurrentScene();

		// Access current scene
		Scene* GetCurrentScene();
		const Scene* GetCurrentScene() const;

		// Update and render current scene
		void Update(float deltaTime);
		void Render(Camera* camera);

	private:
		SceneManager() = default;
		std::unique_ptr<Scene> m_currentScene;
	};
}