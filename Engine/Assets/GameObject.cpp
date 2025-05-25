#include "GameObject.h"
#include "Engine/Graphics/Model.h"
#include "../KSContext.h"

namespace KSEngine
{
	GameObject::GameObject(const std::string& name)
		: m_name(name)
	{
	}

	void GameObject::Update(float /*deltaTime*/)
	{
		// Add per-frame logic here (e.g., animation, scripts)
	}

	void GameObject::Render(Camera* camera)
	{
		if (m_model)
		{
			m_model->Draw(
				KSContext::Instance().GetDirectX12().GetCommandList(),
				camera->GetCBVGPUAddress()
			);
		}
	}
}