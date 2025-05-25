#include "EngineApp.h"
#include "KSContext.h"
#include "Engine/Platform/KSWindow.h"
#include <iostream>
#include <string>
#include "Assets/SceneManager.h"
#include "Logger/KSLogger.h"

namespace KSEngine
{

	EngineApp::EngineApp() = default;
	EngineApp::~EngineApp() = default;

	int32_t EngineApp::Run(const std::wstring& windowTitle, int width, int height)
	{
		int result = Init(windowTitle, width, height);
		if (result != 0)
		{
			std::cout << "Failed to initialize the engine." << std::endl;
			return result;
		}
		OnInitialize();

		while (KSContext::Instance().GetIsRunning())
		{
			if (auto msg = KSWindow::ProcessMessages())
			{
				KSContext::Instance().StopRunning();
				OnShutdown();
				return *msg;
			}

			m_camera.HandleInput();
			m_camera.UpdateConstantBuffer();

			OnUpdate();
			Render();
		}

		OnShutdown();
		return 0;
	}

	int EngineApp::GetWindowWidth() const
	{
		return KSContext::Instance().GetWidth();
	}

	int EngineApp::GetWindowHeight() const
	{
		return KSContext::Instance().GetHeight();
	}


	int EngineApp::Init(const std::wstring& windowTitle, int width, int height)
	{
		// Load config file
		auto& gameConfig = KSContext::Instance().GetGameConfig();
		if (!gameConfig.LoadFromFile("Config/gameconfig.json"))
		{
			// log error to console
			LOG_ERROR(L"Failed to load game config file.");
			return -1;
		}

		if (!gameConfig.LoadMaterialsFromFile("Assets/Materials/Materials.json"))
		{
			LOG_ERROR(L"Failed to load materials from file");
			return -1;
		}

		// Initialize window and graphics context
		KSContext::Instance().Initialize(windowTitle, width, height);

		if (SceneManager::Instance().LoadScene(gameConfig.GetInitialScene()))
		{
			LOG_INFO(L"Initial scene loaded successfully.");
		}
		else
		{
			LOG_ERROR(L"Failed to load initial scene.");
			return -1;
		}


		m_camera.Init(KSEngine::KSContext::Instance().GetAspectRatio());
		m_camera.CreateConstantBuffer();
		return 0;
	}

	void EngineApp::Render()
	{
		auto& dx12 = KSContext::Instance().GetDirectX12();
		auto cmdList = dx12.GetCommandList();

		dx12.BeginFrame();

		D3D12_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(KSContext::Instance().GetWidth());
		viewport.Height = static_cast<float>(KSContext::Instance().GetHeight());
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		D3D12_RECT scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = KSContext::Instance().GetWidth();
		scissorRect.bottom = KSContext::Instance().GetHeight();

		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Call the game's rendering code
		OnRender();
		m_camera.DrawUI();
		SceneManager::Instance().Render(&m_camera);

		dx12.EndFrame();
	}

} // namespace KSEngine