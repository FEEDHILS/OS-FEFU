#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
    printf("Введите название/путь до приложения: ");
    char* input = malloc(sizeof(char) * 256);
    scanf("%s", input);
    if (strlen(input) == 1 && *input == '.')
    {
        #if defined(_WIN32)
        input = "C:\\Windows\\System32\\notepad.exe";
        #elif defined(__linux__)
        input = "firefox";
        #endif
    }
    
    printf("Введите время ожидания приложения (в мс, где -1 - бесконечно) ");
    int waitTime;
    scanf("%d", &waitTime);

    CreateChild(input, waitTime);
    return 0;
}