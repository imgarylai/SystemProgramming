#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t proc = fork();

    if (proc < 0)
    {
        printf("Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        printf("In the child process.\n");
        exit(25);
    }
    else
    {
        printf("In the parent process.\n");

        int status = -1;
        wait(&status);

        printf("The Child Process Returned with %d\n", WEXITSTATUS(status));
    }

    return EXIT_SUCCESS;
}
