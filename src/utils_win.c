#include "include/utils.h"

#include <windows.h>
#include <stdio.h>


int CreateChild(char* path, int waitTime)
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    int error = CreateProcess(
        (LPCSTR)path,
        NULL,
        NULL,
        NULL,
        FALSE, 
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (error != 0)
    {
        WaitForSingleObject(pi.hProcess, waitTime == -1 ? INFINITE : (DWORD)waitTime );

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }
    else
    {
        printf("Не получилось запустить процесс. Код ошибки %d \n", GetLastError());
        return GetLastError();
    }
    
}