#include <windows.h>
#include <fstream>

#include "LCE.h"

typedef Minecraft* (*GetGameInstanceFn)();

void Log(const char* message)
{
    char path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, path);
    strcat_s(path, "\\Project1.asi.log");

    std::ofstream file(path, std::ios::app);
    if (!file)
        return;

    file << message << "\n";
}

DWORD WINAPI MainThread(LPVOID)
{
    // Wait for the game to initialize
    Sleep(5000);
	Log("Alive");

    HMODULE hGame = GetModuleHandle(nullptr);
    if (!hGame)
        return 0;

    auto getGameInstance =
        (GetGameInstanceFn)GetProcAddress(hGame, "GetGameInstance");

    if (!getGameInstance)
        return 0;

    Minecraft* minecraft = nullptr;

	// If minecraft doesnt exist, wait a bit
    while (!minecraft)
    {
        minecraft = getGameInstance();
        Sleep(100);
    }

	// You can replace the below with whatever you want
	char buf[128];
    sprintf_s(buf, "Game ptr: 0x%p", minecraft);
	Log(buf);

    return 0;
}

extern "C" __declspec(dllexport) void InitializeASI()
{
	CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}
