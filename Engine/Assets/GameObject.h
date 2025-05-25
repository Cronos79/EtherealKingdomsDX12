#pragma once
#include <memory>
#include <string>
#include <DirectXMath.h>
#include <Engine/Graphics/Camera.h>

namespace KSEngine
{
	class Model;

	class GameObject
	{
	public:
		struct Transform
		{
			DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
			DirectX::XMFLOAT4 rotation{ 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion (x, y, z, w)
			DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };

			Transform() = default;
		};

		GameObject(const std::string& name);

		// Accessors
		const std::string& GetName() const
		{
			return m_name;
		}
		Transform& GetTransform()
		{
			return m_transform;
		}
		const Transform& GetTransform() const
		{
			return m_transform;
		}

		void SetModel(std::shared_ptr<Model> model)
		{
			m_model = std::move(model);
		}
		std::shared_ptr<Model> GetModel() const
		{
			return m_model;
		}

		// Update/Render hooks (expand as needed)
		void Update(float deltaTime);
		void Render(Camera* camera);

	private:
		std::string m_name;
		Transform m_transform;
		std::shared_ptr<Model> m_model;
	};
}