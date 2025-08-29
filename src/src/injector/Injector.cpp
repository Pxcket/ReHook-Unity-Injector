#include <iostream>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>
#include "../utils/Console.h"
#include "../utils/Process.h"

#pragma comment(lib, "winmm.lib")

const char* SOUND_PATH = ".\\Misc\\success.wav";

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
    set_console_warning();
    std::cout << skCrypt("[*] Waiting for a Unity game to launch...\n");
    reset_console();

    while (true) {
        HWND hwnd = find_unity_window();
        if (hwnd) {
            DWORD pid = get_pid_from_window(hwnd);
            if (pid) {
                std::string exeName = get_process_name(pid);
                std::string title = get_window_title(hwnd);


                std::cout << "\r" << std::string(80, ' ') << "\r";

                set_console_success();
                std::cout << skCrypt("[+] Game Detected!\n");
                std::cout << skCrypt("[*] Executable: ") << exeName << std::endl;
                reset_console();

                return true;
            }
        }
        Sleep(100);
    }
}

bool inject_dll(DWORD processId, const std::wstring& dllPath) {
    if (!std::filesystem::exists(dllPath)) {
        set_console_error();
        std::wcout << skCrypt("[!] Not found: ") << dllPath << std::endl;
        reset_console();
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


bool inject_mods() {
    std::wstring modsDir = std::filesystem::current_path().wstring() + L"\\Misc\\Mods";

    set_console_success();
    std::cout << skCrypt("[+] Scanning for mods...\n");
	Sleep(500);
    reset_console();

    if (!std::filesystem::exists(modsDir)) {
        set_console_error();
        std::wcout << skCrypt("[!] Mods folder missing: \"") << modsDir << L"\"\n";
        std::cout << skCrypt("[!] Create it and place your .dll files inside.\n");
        Sleep(500);
        reset_console();
        return false;
    }

    std::vector<std::wstring> dlls;
    for (const auto& entry : std::filesystem::directory_iterator(modsDir)) {
        if (entry.path().extension() == L".dll") {
            dlls.push_back(entry.path().wstring());
        }
    }

    switch (dlls.size()) {
    case 0:
        set_console_error();
        std::cout << skCrypt("[-] No mods found. Place .dll files in ./Misc/Mods/\n");
        reset_console();
        return false;
    case 1:
        set_console_success();
        std::cout << skCrypt("[+] Found 1 mod.\n");
        Sleep(500);
        break;
    default:
        set_console_success();
        std::cout << skCrypt("[+] Found ") << dlls.size() << skCrypt(" mods.\n");
        Sleep(500);
        break;
    }
    reset_console();


    if (!wait_for_unity_game()) {
        return false;
    }

    HWND hwnd = find_unity_window();
    if (!hwnd) {
        set_console_error();
        std::cout << skCrypt("[!] Game window vanished!\n");
        Sleep(500);
        reset_console();
        return false;
    }

    DWORD pid = get_pid_from_window(hwnd);
    if (!pid) {
        set_console_error();
        std::cout << skCrypt("[!] Failed to get process ID.\n");
        Sleep(500);
        reset_console();
        return false;
    }

    std::string exeName = get_process_name(pid);

    set_console_success();
    std::cout << skCrypt("[+] Target: ") << exeName << " (PID: " << pid << ")" << std::endl;
    Sleep(500);
    std::cout << skCrypt("[*] Injecting mods...\n");
    Sleep(500);
    reset_console();

    int injected = 0;
    for (const auto& dll : dlls) {
        if (inject_dll(pid, dll)) {
            set_console_success();
            std::wcout << skCrypt("[+] Injected: ") << dll << std::endl;

            PlaySoundA(SOUND_PATH, NULL, SND_FILENAME | SND_ASYNC);

            injected++;
        }
        else {
            set_console_error();
            std::wcout << skCrypt("[!] Failed: ") << dll << std::endl;
        }
        reset_console();
        Sleep(50);
    }

    set_console_success();
    std::cout << skCrypt("[+] ") << injected << skCrypt(" / ") << dlls.size() << skCrypt(" mods injected.\n");
    reset_console();

    return true;
}