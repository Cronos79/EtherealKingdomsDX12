#pragma once
#include <string>
#include <cstdint>
#include "Assets/GameConfig.h"
#include "Graphics/Camera.h"

namespace KSEngine
{

	class EngineApp {
	public:
		EngineApp();
		virtual ~EngineApp();

		// Entry point for the app
		int32_t Run(const std::wstring& windowTitle, int width, int height);

	protected:
		// Hooks for the game to override
		virtual void OnInitialize()
		{
		}
		virtual void OnUpdate()
		{
		}
		virtual void OnRender()
		{
		}
		virtual void OnShutdown()
		{
		}

		// Accessors for window/context if needed
		int GetWindowWidth() const;
		int GetWindowHeight() const;

	protected:
		int Init(const std::wstring& windowTitle, int width, int height);
		void Render();


	protected:		
		Camera m_camera;
	};

} // namespace KSEngine