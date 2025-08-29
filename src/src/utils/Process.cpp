#include <windows.h>      
#include <tlhelp32.h>
#include "Process.h"

DWORD get_process_id_by_name(const char* processName) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    BOOL result = Process32First(hSnapshot, &pe32);
    while (result) {
#ifdef UNICODE

        WCHAR wProcessName[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, processName, -1, wProcessName, MAX_PATH);
        if (_wcsicmp(pe32.szExeFile, wProcessName) == 0) {
#else
        if (_stricmp(pe32.szExeFile, processName) == 0) {
#endif
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
        result = Process32Next(hSnapshot, &pe32);
    }

    CloseHandle(hSnapshot);
    return 0;
}