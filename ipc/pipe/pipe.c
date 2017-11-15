#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

static int fd[2];

void throw_int(int i)
{
    write(fd[1], &i, sizeof(int));
}

int catch_int()
{
    int result = -1;
    read(fd[0], &result, sizeof(int));

    return result;
}

int main()
{
    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Error: Unable to create pipe.\n");
        exit(EXIT_FAILURE);
    }
    
    throw_int(100);
    printf("%d\n", catch_int());

    return EXIT_SUCCESS;
}
