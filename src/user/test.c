#include "stdio.h"
#include "stdlib.h"

int main(int argc, chr* argv[])
{
    int i = 0;
    printf("Hello World!\n");
    printf("argc:%d, argv:%#018lx\n", argc, argv);
    for(i = 0; i < argc; i++)
        printf("argv[%d]:%s\n", i, argv[i]);

    exit(0)
    return 0;
}