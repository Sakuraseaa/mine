#include <stdio.h>

int main(int argc, char* argv[]) {
    long *p = (long*)argv;

    printf("argc:%d\n",*(p - 1));
    while (*p)
    {
        printf("%s\n", *p);
        p++;
    }
    return 0;
}