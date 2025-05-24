#include "GameApp.h"
#include "Engine/KSContext.h"
#include "imgui.h"

GameApp::GameApp() = default;
GameApp::~GameApp() = default;

void GameApp::Initialize()
{
	// Initialize the window and DirectX 12
	KSEngine::KSContext::Instance().Initialize(L"Kingdom game", 1920, 1080);

	// Setup camera
	m_camera.Init(KSEngine::KSContext::Instance().GetAspectRatio());
	m_camera.CreateConstantBuffer();

	// Load test model
	AddModel("C:/Projects/KingdomSim/Models/Test.fbx");
}

int32_t GameApp::Run()
{
	while (KSEngine::KSContext::Instance().GetIsRunning())
	{
		if (auto msg = KSEngine::KSWindow::ProcessMessages())
		{
			KSEngine::KSContext::Instance().StopRunning();
			return *msg;
		}
		RenderFrame();
	}
	return 0;
}

void GameApp::AddModel(const std::string& filename)
{
	auto& dx12 = KSEngine::KSContext::Instance().GetDirectX12();
	auto device = dx12.GetDevice();
	auto model = std::make_unique<KSEngine::Model>();
	model->LoadFromFile(device, filename);
	m_models.push_back(std::move(model));
}

void GameApp::RenderFrame()
{
	auto& dx12 = KSEngine::KSContext::Instance().GetDirectX12();
	auto cmdList = dx12.GetCommandList();

	dx12.BeginFrame();

	m_camera.DrawUI();
	m_camera.UpdateConstantBuffer();
	m_camera.HandleInput();

	// Set viewport and scissor
	D3D12_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(KSEngine::KSContext::Instance().GetWidth());
	viewport.Height = static_cast<float>(KSEngine::KSContext::Instance().GetHeight());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = KSEngine::KSContext::Instance().GetWidth();
	scissorRect.bottom = KSEngine::KSContext::Instance().GetHeight();

	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Draw all models
	for (const auto& model : m_models)
	{
		if (model)
			model->Draw(cmdList, m_camera.GetCBVGPUAddress());
	}

	dx12.EndFrame();
}