#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>

typedef struct{
    pthread_mutex_t mutex;
    int counter;
} SharedData;

SharedData* pointer;
void Execute(char* arg)
{
    int shmem = shm_open("/MyMemCounter", O_CREAT | O_EXCL | O_RDWR, 0666);
    if (errno != EEXIST)
    {
        ftruncate(shmem, sizeof(SharedData));
        pointer = (SharedData *)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0);
        pointer->counter = 0;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        int error = pthread_mutex_init(&pointer->mutex, &attr);
    }
    else
    {
        int shmem = shm_open("/MyMemCounter", O_RDWR, 0);
        pointer = (SharedData *)mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shmem, 0);
    }
    
    pointer->counter += 1;
    nanosleep((timespec){0, 300000000L}, NULL);

    
}