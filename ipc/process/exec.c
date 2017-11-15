#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int main()
{
    execlp("ls", "ls", "-al", (char *)0);

    return EXIT_SUCCESS;
}
