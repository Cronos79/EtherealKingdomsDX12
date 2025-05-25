#pragma once
#include "Engine/EngineApp.h"
#include "Engine/Graphics/Camera.h"
#include "Engine/Graphics/Model.h"
#include <vector>
#include <memory>

class GameApp : public KSEngine::EngineApp {
public:
	GameApp();
	~GameApp() override;

protected:
	void OnInitialize() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;
};