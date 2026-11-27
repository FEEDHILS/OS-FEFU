#include "include/utils.h"

#include <windows.h>
#include <stdio.h>
#include <process.h>

int* pointer;

void Fork(PROCESS_INFORMATION* pi, char* cmdattr);

HANDLE logmutex = INVALID_HANDLE_VALUE;
void Log()
{
    HANDLE log_file = CreateFile("./log.txt", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WaitForSingleObject(logmutex, INFINITE);
    char line[512];

    SYSTEMTIME t;
    GetLocalTime(&t);

    int length = snprintf(
        line, sizeof line,
        "%04u-%02u-%02u %02u:%02u:%02u.%03u pid=%lu %d \n",
        t.wYear, t.wMonth, t.wDay,
        t.wHour, t.wMinute, t.wSecond, t.wMilliseconds,
        (unsigned long)GetCurrentProcessId(),
        *pointer
    );

    WriteFile(log_file, line, length, NULL, NULL);
    ReleaseMutex(logmutex);
}

void __cdecl PrintThread(void* args)
{
    while(1)
    {
        Sleep(1000);
        // snprintf("Текущее значение счетчика")
        Log();
        // printf("Текущее значение счетчика: %d \n", *pointer);
    }
}

void __cdecl IncrementThread(void* args)
{
    HANDLE counterMutex = CreateMutexA(NULL, FALSE, "Local\\MyCounterMutex");
    while(1)
    {
        Sleep(300);
        WaitForSingleObject(counterMutex, INFINITE);
        *pointer += 1;
        ReleaseMutex(counterMutex);
    }
}

void __cdecl CopyThread(void* args)
{
    // Токо 1 процесс может создавать копии, по необходимости ждем своей очереди.
    HANDLE mutex = CreateMutex(NULL, FALSE, "Local\\MasterMutex");
    DWORD result = WaitForSingleObject(mutex, INFINITE);


    // char FileName[MAX_PATH];
    // GetModuleFileName(NULL, FileName, MAX_PATH);

    if (result == WAIT_OBJECT_0 || result == WAIT_ABANDONED)
    {
        Sleep(3000);
        
        PROCESS_INFORMATION pi1 = {0};
        Fork(&pi1, "1");

        PROCESS_INFORMATION pi2 ;
        Fork(&pi2, "2");

        while(1)
        {
            Sleep(3000);
            HANDLE processes[] = { pi1.hProcess, pi2.hProcess };
            DWORD result = WaitForMultipleObjects(2, processes, TRUE, 0);
            if (result == WAIT_OBJECT_0)
            {
                CloseHandle(pi1.hProcess);
                CloseHandle(pi2.hProcess);
                
                Fork(&pi1, "1");
                Fork(&pi2, "2");
            }

            else
            {
                printf("Дочерние процессы еще не завершили выполнение \n");
            }
        }

    }

}


int CreateSharedMem(char* Name, HANDLE* shmem)
{
    *shmem = CreateFileMapping(INVALID_HANDLE_VALUE,
                    NULL,
                    PAGE_READWRITE,
                    0,
                    sizeof(int),
                    Name
                );

    if (*shmem == NULL)
        return GetLastError();

    // return GetLastError() ? GetLastError() == ERROR_ALREADY_EXISTS : 0;
    return 0;
}


char FileName[MAX_PATH];
void Fork(PROCESS_INFORMATION* pi, char* cmdattr)
{
    GetModuleFileName(NULL, FileName, MAX_PATH);
    ZeroMemory(pi, sizeof(PROCESS_INFORMATION));

    STARTUPINFO si1 = {0}; si1.cb = sizeof(si1);
    CreateProcess(FileName, cmdattr,
            NULL, NULL,
            FALSE, 0,
            NULL, NULL,
            &si1, pi);
}

void Copy1()
{
    printf("Копия 1\n");
    HANDLE counterMutex = CreateMutexA(NULL, FALSE, "Local\\MyCounterMutex");
    WaitForSingleObject(counterMutex, INFINITE);
    *pointer += 10;

    ReleaseMutex(counterMutex);
}

void Copy2()
{
    HANDLE counterMutex = CreateMutexA(NULL, FALSE, "Local\\MyCounterMutex");
    WaitForSingleObject(counterMutex, INFINITE);
    printf("Копия 2\n");

    *pointer *= 2;
    ReleaseMutex(counterMutex);
    Sleep(2000);
    WaitForSingleObject(counterMutex, INFINITE);
    *pointer /= 2;
    ReleaseMutex(counterMutex);
}


void Execute(char* arg[])
{
    HANDLE shmem;
    if (CreateSharedMem("Local\\MyCounter", &shmem) != 0)
    {
        printf("Не получилось создать общую память\n");
        exit(1);
    }

    int isCreator = GetLastError() != ERROR_ALREADY_EXISTS;

    pointer = (int*) MapViewOfFile(shmem, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(int));

    if (pointer == NULL)
    {
        printf("Не получилось достать объекты в общей памяти");
        exit(1);
    }

    logmutex = CreateMutex(NULL, FALSE, "Local\\MyLogMutex");
    if (isCreator)
    {
        *pointer = 0;
        printf("Инициализирую общую память\n");
    }

    if (strcmp(arg[0], "1") == 0)
    {
        Copy1();
        return;
    }
    else if (strcmp(arg[0], "2") == 0)
    {
        Copy2();
        return;
    }

    HANDLE printThread = (HANDLE) _beginthread(PrintThread, 0, NULL);
    HANDLE incrementThread = (HANDLE) _beginthread(IncrementThread, 0, NULL);
    HANDLE copyThread = (HANDLE) _beginthread(CopyThread, 0, NULL);

    HANDLE counterMutex = CreateMutexA(NULL, FALSE, "Local\\MyCounterMutex");
    int answ;
    do
    {
        scanf("%d", &answ);
        WaitForSingleObject(counterMutex, INFINITE);
        *pointer = answ;
        ReleaseMutex(counterMutex);
    } while (answ != -1);
}