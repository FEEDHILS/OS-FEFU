#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"

int CreateChild(char* path, int waitUntil)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(path, (char *)NULL)
    }

    int status;
    waitpid(pid, &status, 0);
    return status;
}