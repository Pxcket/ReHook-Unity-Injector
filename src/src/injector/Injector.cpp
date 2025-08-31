#include "injector.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>
#include "../utils/Console.h"
#include "../utils/Process.h"
#pragma comment(lib, "winmm.lib")

const char* SOUND_PATH = ".\\Misc\\success.wav";

bool Injector::CreateDirectorys() {
    std::wstring miscDir = std::filesystem::current_path().wstring() + L"\\Misc";
    std::wstring modsDir = miscDir + L"\\Mods";

    bool createdAny = false;

    if (!std::filesystem::exists(miscDir)) {
        ConsoleColors::Warning();
        std::cout << skCrypt("[*] Misc directory not found, generating...\n");
        ConsoleColors::Reset();

        if (!std::filesystem::create_directory(miscDir)) {
            ConsoleColors::Error();
            std::wcout << skCrypt("[!] Failed to create Misc directory: ") << miscDir << std::endl;
            ConsoleColors::Reset();
            return false;
        }
        createdAny = true;
    }

    if (!std::filesystem::exists(modsDir)) {
        ConsoleColors::Warning();
        std::cout << skCrypt("[*] Mods directory not found, generating...\n");
        ConsoleColors::Reset();

        if (!std::filesystem::create_directory(modsDir)) {
            ConsoleColors::Error();
            std::wcout << skCrypt("[!] Failed to create Mods directory: ") << modsDir << std::endl;
            ConsoleColors::Reset();
            return false;
        }
        createdAny = true;
    }

    if (createdAny) {
        ConsoleColors::Success();
        std::cout << skCrypt("[+] Required directories generated successfully!\n");
        std::cout << skCrypt("[+] Please add your mod DLLs to ./Misc/Mods/ and restart the injector.\n");
        ConsoleColors::Reset();
        Sleep(2000);
        return false; 
    }

    return true;
}
HWND find_unity_window() {
    HWND hwnd = nullptr;
    do {
        hwnd = FindWindowExA(nullptr, hwnd, "UnityWndClass", nullptr);
        if (hwnd && IsWindowVisible(hwnd)) {
            WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
            GetWindowPlacement(hwnd, &wp);
            if (wp.showCmd != SW_SHOWMINIMIZED) {
                return hwnd;
            }
        }
    } while (hwnd != nullptr);
    return nullptr;
}

DWORD get_pid_from_window(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    return pid;
}

std::string get_process_name(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!hProcess) return "Unknown";

    char exeName[MAX_PATH] = { 0 };
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameA(hProcess, 0, exeName, &size)) {
        char* baseName = strrchr(exeName, '\\');
        CloseHandle(hProcess);
        return baseName ? std::string(baseName + 1) : std::string(exeName);
    }

    CloseHandle(hProcess);
    return "Unknown";
}

std::string get_window_title(HWND hwnd) {
    char title[512] = { 0 };
    GetWindowTextA(hwnd, title, sizeof(title));
    return std::string(title);
}

bool wait_for_unity_game() {
    ConsoleColors::Warning();
    std::cout << skCrypt("[*] Waiting for a Unity game to launch...\n");
    ConsoleColors::Reset();

    while (true) {
        HWND hwnd = find_unity_window();
        if (hwnd) {
            DWORD pid = get_pid_from_window(hwnd);
            if (pid) {
                std::string exeName = get_process_name(pid);
                std::string title = get_window_title(hwnd);

                std::cout << "\r" << std::string(80, ' ') << "\r";

                ConsoleColors::Success();
                std::cout << skCrypt("[+] Game Detected!\n");
                std::cout << skCrypt("[*] Executable: ") << exeName << std::endl;
                ConsoleColors::Reset();

                return true;
            }
        }
        Sleep(100);
    }
}

bool inject_dll(DWORD processId, const std::wstring& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        ConsoleColors::Error();
        std::wcout << skCrypt("[!] Not found: ") << dllPath << std::endl;
        ConsoleColors::Reset();
        return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) return false;

    size_t pathSize = (dllPath.length() + 1) * sizeof(wchar_t);
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, nullptr, pathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), pathSize, nullptr)) {
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "LoadLibraryW"),
        pRemoteMemory, 0, nullptr);

    bool success = (hThread != nullptr);
    if (success) {
        WaitForSingleObject(hThread, 5000);
        CloseHandle(hThread);
    }

    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return success;
}

bool Injector::LoadMods() {
    if (!Injector::CreateDirectorys()) {
        return false;
    }

    std::wstring modsDir = std::filesystem::current_path().wstring() + L"\\Misc\\Mods";

    ConsoleColors::Success();
    std::cout << skCrypt("[+] Scanning for mods...\n");
    Sleep(500);
    ConsoleColors::Reset();

    std::vector<std::wstring> dlls;
    for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
        if (entry.path().extension() == L".dll") {
            dlls.push_back(entry.path().wstring());
        }
    }

    switch (dlls.size()) {
    case 0:
        ConsoleColors::Warning();
        std::cout << skCrypt("[!] No mods found. Place .dll files in ./Misc/Mods/\n");
        std::cout << skCrypt("[!] The program will close and wait for you to add mods.\n");
        ConsoleColors::Reset();
        Sleep(3000);
        return false;
    case 1:
        ConsoleColors::Success();
        std::cout << skCrypt("[+] Found 1 mod.\n");
        Sleep(500);
        break;
    default:
        ConsoleColors::Success();
        std::cout << skCrypt("[+] Found ") << dlls.size() << skCrypt(" mods.\n");
        Sleep(500);
        break;
    }
    ConsoleColors::Reset();

    if (!wait_for_unity_game()) {
        return false;
    }

    HWND hwnd = find_unity_window();
    if (!hwnd) {
        ConsoleColors::Error();
        std::cout << skCrypt("[!] Game window vanished!\n");
        Sleep(500);
        ConsoleColors::Reset();
        return false;
    }

    DWORD pid = get_pid_from_window(hwnd);
    if (!pid) {
        ConsoleColors::Error();
        std::cout << skCrypt("[!] Failed to get process ID.\n");
        Sleep(500);
        ConsoleColors::Reset();
        return false;
    }

    std::string exeName = get_process_name(pid);

    ConsoleColors::Success();
    std::cout << skCrypt("[+] Target: ") << exeName << " (PID: " << pid << ")" << std::endl;
    Sleep(500);
    std::cout << skCrypt("[*] Injecting mods...\n");
    Sleep(500);
    ConsoleColors::Reset();

    int injected = 0;
    for (const auto& dll : dlls) {
        if (inject_dll(pid, dll)) {
            ConsoleColors::Success();
            std::wcout << skCrypt("[+] Injected: ") << dll << std::endl;

            PlaySoundA(SOUND_PATH, NULL, SND_FILENAME | SND_ASYNC);

            injected++;
        }
        else {
            ConsoleColors::Error();
            std::wcout << skCrypt("[!] Failed: ") << dll << std::endl;
        }
        ConsoleColors::Reset();
        Sleep(50);
    }

    ConsoleColors::Success();
    std::cout << skCrypt("[+] ") << injected << skCrypt(" / ") << dlls.size() << skCrypt(" mods injected.\n");
    ConsoleColors::Reset();

    return true;
}


