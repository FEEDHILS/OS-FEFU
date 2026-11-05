#include <stdio.h>
#include <stdlib.h>

#include "include/utils.h"

int main(void)
{
    CreateChild("C:\\Windows\\System32\\notepad.exe", -1);
    printf("hello world?");
    return 0;
}