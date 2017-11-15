#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <unistd.h>

int main()
{
    printf("Let's invoke ls command!\n\n");

    pid_t proc = fork();

    if (proc < 0)
    {
        printf("Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        if (execlp("ls", "ls", "-al", (char *)0) == -1)
        {
            printf("Error: Unable to load the executable.\n");
            exit(EXIT_FAILURE);
        }

        /* NEVER REACHED */
    }
    else
    {
        int status = -1;
        wait(&status);

        printf("\nThe exit code of ls is %d.\n", WEXITSTATUS(status));
    }

    return EXIT_SUCCESS;
}
