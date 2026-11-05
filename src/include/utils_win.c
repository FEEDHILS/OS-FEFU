#include <windows.h>

#include "utils.h"

int CreateChild(char* path, int waitUntil)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    CreateProcess(
        (LPCSTR)path,
        NULL,
        NULL,
        NULL,
        FALSE, // Не наследуем дескриптор
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
    WaitForSingleObject(pi.hProcess, waitUntil == -1 ? INFINITE : (DWORD)waitUntil );

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}