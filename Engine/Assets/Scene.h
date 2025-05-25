#pragma once
#include <vector>
#include <memory>
#include <string>
#include <Engine/Graphics/Camera.h>

namespace KSEngine
{
	class GameObject;

	class Scene
	{
	public:
		Scene(const std::string& name);

		// Add a GameObject to the scene
		void AddObject(std::shared_ptr<GameObject> obj);

		// Find a GameObject by name (returns nullptr if not found)
		std::shared_ptr<GameObject> FindObject(const std::string& name) const;

		// Update and render all objects
		void Update(float deltaTime);
		void Render(Camera* camera);

		// Accessors
		const std::string& GetName() const
		{
			return m_name;
		}
		const std::vector<std::shared_ptr<GameObject>>& GetObjects() const
		{
			return m_objects;
		}

		// Remove all objects
		void Clear();

	private:
		std::string m_name;
		std::vector<std::shared_ptr<GameObject>> m_objects;
	};
}