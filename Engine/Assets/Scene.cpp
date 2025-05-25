#include "Scene.h"
#include "GameObject.h"

namespace KSEngine
{
	Scene::Scene(const std::string& name)
		: m_name(name)
	{
	}

	void Scene::AddObject(std::shared_ptr<GameObject> obj)
	{
		m_objects.push_back(std::move(obj));
	}

	std::shared_ptr<GameObject> Scene::FindObject(const std::string& name) const
	{
		for (const auto& obj : m_objects)
		{
			if (obj && obj->GetName() == name)
				return obj;
		}
		return nullptr;
	}

	void Scene::Update(float deltaTime)
	{
		for (auto& obj : m_objects)
		{
			if (obj) obj->Update(deltaTime);
		}
	}

	void Scene::Render(Camera* camera)
	{
		for (auto& obj : m_objects)
		{
			if (obj) obj->Render(camera);
		}
	}

	void Scene::Clear()
	{
		m_objects.clear();
	}
}