#include "Engine/Platform/WinInclude.h"
#include "Game/GameApp.h"

int main(int argc, char* argv[])
{
	// Call WinMain
	return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	GameApp app;
	app.Initialize();
	app.Run();

	return 0;
}