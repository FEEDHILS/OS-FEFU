#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void)
{
    printf("Введите название/путь до приложения: ");
    char* input = malloc(sizeof(char) * 256);
    char* args_ = malloc(sizeof(char) * 256);

    int args_len = scanf("%s %s", input, args_);

    // char* args[] = strip(args, args_len - 1);   
    char* args[] = {NULL}; 
    if (strlen(input) == 1 && *input == '.')
    {
        #if defined(_WIN32)
        input = "C:\\Windows\\System32\\notepad.exe";
        #elif defined(__linux__)
        input = "firefox";
        #endif
    }
    
    printf("Введите время ожидания приложения (в мс, где -1 - бесконечно)");
    int waitTime;
    scanf("%d", &waitTime);

    CreateChild(input, args, waitTime);
    return 0;
}

// Потом...
// char** strip(char* str, int amount)
// {
//     char* arr[] = malloc( sizeof(char)*16 * amount );
//     while( strchr(str, (int)' ') != NULL )
//     {

//     }
// }