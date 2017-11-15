#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>


int main(int argc, char **argv)
{
    /* -- Check Arguments -------- */
    if (argc < 4)
    {
        fprintf(stderr, "Error: Incorrect Arguments.\n");
        fprintf(stderr, "Usage: $ red STDIN_FILE STDOUT_FILE EXECUTABLE ARGS\n");
        exit(EXIT_FAILURE);
    }


    /* -- Open the Redirect Files -------- */

    int fd_in = open(argv[1], O_RDONLY);

    if (fd_in < 0)
    {
        fprintf(stderr, "Error: Unable to open the input file.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd_out < 0)
    {
        fprintf(stderr, "Error: Unable to open the output file.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }


    /* -- Replace the Executable Image of the Process -------- */
    if (execvp(argv[3], argv + 3) == -1)
    {
        fprintf(stderr, "Error: Unable to load executable image.\n");
        exit(EXIT_FAILURE);
    }


    return EXIT_SUCCESS;
}
