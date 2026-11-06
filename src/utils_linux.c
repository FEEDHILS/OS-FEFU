#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>


int CreateChild(char* path, int waitTime)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        printf("Не получилось дублировать процесс. Код ошибки %d \n", errno);
        return errno;
    }
    elif (pid == 0)
    {
        if (execlp(path, "", NULL) == -1)
        {
            printf("Не получилось заменить процесс. Код ошибки %d \n", errno);
            return errno;
        }
    }

    if (waitTime != 0)
    {
        int status;
        waitpid(pid, &status, 0);
        if (status != -1)
            return 0;
        else
            return errno;
    }
    else
        return 0;
}