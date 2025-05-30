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
	app.Run(L"Kingdom game", 1920, 1080);

	return 0;
}