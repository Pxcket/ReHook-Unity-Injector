#include <windows.h>
#include <iostream>
#include "core/Logo.h"
#include "utils/LoadingBar.h"
#include "injector/Injector.h"
#include "utils/Console.h"
#include <shellapi.h>
#include <filesystem>
#pragma comment(lib, "shell32.lib")
#define skCrypt(x) x  

bool ForceAdmin(); //Forward declaration for sigma layout
void SetConsoleOpacity(BYTE opacity);

int main() {
    if (!ForceAdmin()) {
        std::cout << skCrypt("[!] Failed to start as admin.\n");
        Sleep(3000);
        return 1;
    }

    SetConsoleOpacity(230);
    SetConsoleTitleA("Unity Mod Injector - by Pxcket & Z0mb1e");
    system("cls");
    Logo::Show();
    loading_bar(2);

    if (!Injector::CreateDirectorys()) {
        ConsoleColors::Error();
        std::cout << skCrypt("[!] Failed to create required directories.\n");
        Sleep(3000);
        return 1;
    }

    std::wstring modsDir = std::filesystem::current_path().wstring() + L"\\Misc\\Mods";
    bool hasMods = false;

    for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
        if (entry.path().extension() == L".dll") {
            hasMods = true;
            break;
        }
    }

    if (!hasMods) {
        ConsoleColors::Warning();
        std::cout << skCrypt("[!] No mods found in ./Misc/Mods/\n");
        std::cout << skCrypt("[!] Please add your mod DLLs and restart the injector.\n");
        ConsoleColors::Reset();
        std::cout << skCrypt("[+] Closing in 5 seconds. Bye-Bye!\n");
        Sleep(5000);
        return 0;
    }

    if (!Injector::LoadMods()) {
        ConsoleColors::Error();
        std::cout << skCrypt("[!] Mod loading failed.\n");
    }
    else {
        ConsoleColors::Success();
        std::cout << skCrypt("[+] Injection completed successfully!\n");
    }

    ConsoleColors::Magenta();
    std::cout << skCrypt("[+] Closing in 5 seconds. Bye-Bye!\n");
    ConsoleColors::Reset();

    Sleep(5000);
    return 0;
}


bool IsUserAnAdmin() {
    BOOL fRet = FALSE;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) { //idiot way but it works :skull:
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) CloseHandle(hToken);
    return fRet;
}

bool ForceAdmin() {
    if (IsUserAnAdmin()) {
        return true; 
    }


    HWND hConsole = GetConsoleWindow();
    if (hConsole) {
        ShowWindow(hConsole, SW_HIDE); 
    }


    char szPath[MAX_PATH];
    if (!GetModuleFileNameA(nullptr, szPath, MAX_PATH)) {
        return false;
    }


    SHELLEXECUTEINFOA sei = { sizeof(sei) };
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "runas";           
    sei.lpFile = szPath;
    sei.nShow = SW_NORMAL;          

    if (!ShellExecuteExA(&sei)) {
        DWORD err = GetLastError();
        if (err == ERROR_CANCELLED) {
            MessageBoxA(nullptr, "Administrator access is required to run this Injector.", "Access Denied", MB_ICONERROR);
        }
        else {
            MessageBoxA(nullptr, "Failed to elevate privileges.", "Error", MB_ICONERROR);
        }
        return false;
    }

    WaitForSingleObject(sei.hProcess, 5000);
    CloseHandle(sei.hProcess);

    exit(0);
    return true;
}

void SetConsoleOpacity(BYTE opacity) {
    HWND hwnd = GetConsoleWindow();
    if (hwnd) {
        LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, opacity, LWA_ALPHA);
    }
}