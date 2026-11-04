#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

int main(void)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    char* path = "C:\\Windows\\System32\\notepad.exe";  
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
    WaitForSingleObject(pi.hProcess, INFINITE );
    printf("hello world?");

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}