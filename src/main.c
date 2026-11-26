#include "include/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[])
{
    char* arg = NULL;
    if (argc > 0)
        arg = argv[0];

    Execute(arg);
    return 0;
}