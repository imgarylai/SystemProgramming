#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };


int main()
{
    int pipe_fd[2];

    if (pipe(pipe_fd) == -1)
    {
        fprintf(stderr, "Error: Unable to create pipe.\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid;

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Error: Unable to fork process.\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        /* -- In the Child Process -------- */

        /* Close Read End */
        close(pipe_fd[0]); /* close read end, since we don't need it. */

        /* My Random Number Generator */
        srand(time(NULL));

        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            sleep(1); // wait 1 second

            int randnum = rand() % 100;
            write(pipe_fd[1], &randnum, sizeof(int));
        }

        exit(EXIT_SUCCESS);
    }
    else
    {
        /* -- In the Parent Process -------- */

        /* Close Write End */
        close(pipe_fd[1]); /* Close write end, since we don't need it. */

        int i;
        for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
        {
            int gotnum;
            read(pipe_fd[0], &gotnum, sizeof(int));

            printf("got number : %d\n", gotnum);
        }
    }

    return EXIT_SUCCESS;
}
