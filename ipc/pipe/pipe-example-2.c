#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };

int main()
{
    /* -- Prepare Pipe -------- */
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1)
    {
        fprintf(stderr, "Error: Unable to create pipe.\n");
        exit(EXIT_FAILURE);
    }


    /* -- Create Child Process -------- */
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Error: Unable to create child process.\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) /* In Child Process */
    {
        /* Close Read End */
        close(pipe_fd[0]); /* Close read end, since we don't need it. */

        /* Bind Write End to Standard Out */
        dup2(pipe_fd[1], STDOUT_FILENO);

        /* Close pipe_fd[1] File Descriptor */
        close(pipe_fd[1]);
        /* Notice: STDOUT_FILENO still remain going to the pipe. */

        /* Load Another Executable */
        execl("random-gen", "./random-gen", (char *)0);

        /* This Process Should Never Go Here */
        fprintf(stderr, "Error: Unexcept flow of control.\n");
        exit(EXIT_FAILURE);
    }
    else /* In Parent Process */
    {
        /* Close pipe_fd[1] File Descriptor */
        close(pipe_fd[1]); /* Close write end, since we will not use it. */

        /* Read Random Number From Pipe */
        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            int gotnum = -1;
            read(pipe_fd[0], &gotnum, sizeof(int));

            printf("got number : %d\n", gotnum);
        }
    }

    return EXIT_SUCCESS;
}
