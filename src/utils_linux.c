#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


int CreateChild(char* path, char* args[], int waitTime)
{

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("Не получилось дублировать процесс. Код ошибки %d \n", errno);
        return errno;
    }
    else if (pid == 0)
    {
        if (execvp(path, args) == -1)
        {
            printf("Не получилось заменить процесс. Код ошибки %d \n", errno);
            _exit(127);
        }
    }

    return 0;
}

int ChildWait(pid_t pid)
{
    int status;
    pid_t s;
    do
    {
        s =  waitpid(pid, &status, 0);
    }
    while(s < 0 && errno == EINTR);

    if (s < 0)
    {
        printf("Ошибка при ожидании. Код ошибки: %d", errno);
    }

    if (WIFEXITED(status))
    {
        printf("Код выхода %d", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    else
        return -1;
}