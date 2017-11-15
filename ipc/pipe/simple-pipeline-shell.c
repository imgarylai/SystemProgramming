#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>



#define CMD_SEQ_BUFF_SIZE (1024)

#define MAX_ARG_COUNT (15)
#define MAX_CMD_COUNT (5)

#define CONFLICT_PREVENT_WORDS "SPPLSH253"

static char const * const pipeline_tmp_[] =
{
    "/tmp/__" CONFLICT_PREVENT_WORDS "_pipeline0.tmp",
    "/tmp/__" CONFLICT_PREVENT_WORDS "_pipeline1.tmp",
    "/tmp/__" CONFLICT_PREVENT_WORDS "_pipeline2.tmp",
    "/tmp/__" CONFLICT_PREVENT_WORDS "_pipeline3.tmp"
};




/* Purpose: Load a command sequence from standard input. */
char *read_cmd_seq();

/* Purpose: Convert a string to argv array array. */
char ***parse_cmd_seq(char *);

/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs);

/* Purpose: Strip to whitespace at the front and end of the string. */
char *str_strip(char *str);

/* Purpose: Count the fequency of a character in the string. */
int str_char_count(char const *str, char c);



int main()
{
    while (1)
    {
        char *cmd_seq = read_cmd_seq();
        if (cmd_seq == NULL) { break; }

        execute_cmd_seq(parse_cmd_seq(cmd_seq));
        fputc('\n', stdout);
    }

    return EXIT_SUCCESS;
}



/* Purpose: Create child process and redirect io.
 * (Only available for execute_cmd_seq().)
 */
void creat_proc(char **argv, int fd_in, int fd_out);

/* Purpose: Create several child process and redirect the standard output
 * to the standard input of the later process.
 */
void execute_cmd_seq(char ***argvs)
{
    int C;
    for (C = 0; C <= MAX_CMD_COUNT; ++C)
    {
        char **argv = argvs[C];
        if (!argv) { break; }

        int fd_in = STDIN_FILENO;
        int fd_out = STDOUT_FILENO;

        if (C > 0)
        {
            fd_in = open(pipeline_tmp_[C - 1], O_RDONLY);

            if (fd_in == -1)
            {
                fprintf(stderr, "Error: Unable to open pipeline tmp r.\n");
                exit(EXIT_FAILURE);
            }
        }

        if (C < MAX_CMD_COUNT && argvs[C + 1] != NULL)
        {
            fd_out = open(pipeline_tmp_[C],
                          O_WRONLY | O_CREAT | O_TRUNC,
                          0644);

            if (fd_out == -1)
            {
                fprintf(stderr, "Error: Unable to open pipeline tmp w.\n");
                exit(EXIT_FAILURE);
            }
        }

        creat_proc(argv, fd_in, fd_out);

        if (fd_in != STDIN_FILENO) { close(fd_in); }
        if (fd_out != STDOUT_FILENO) { close(fd_out); }
    }
}



/* Purpose: Create child process and redirect io. */
void creat_proc(char **argv, int fd_in, int fd_out)
{
    pid_t proc = fork();

    if (proc < 0)
    {
        fprintf(stderr, "Error: Unable to fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (proc == 0)
    {
        if (fd_in != STDIN_FILENO)
        {
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (fd_out != STDOUT_FILENO)
        {
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr,
                    "Error: Unable to load the executable %s.\n",
                    argv[0]);

            exit(EXIT_FAILURE);
        }

        /* NEVER REACH */
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        wait(&status);
    }
}



/* Purpose: Load a command sequence from standard input. */
char *read_cmd_seq()
{
    static char cmd_seq_buffer[CMD_SEQ_BUFF_SIZE];

    fputs("<simple-shell>$ ", stdout);
    fflush(stdout);

    memset(cmd_seq_buffer, '\0', sizeof(cmd_seq_buffer));
    fgets(cmd_seq_buffer, sizeof(cmd_seq_buffer), stdin);

    if (feof(stdin)) { return NULL; }
    
    char *cmd_seq = str_strip(cmd_seq_buffer);
    if (strlen(cmd_seq) == 0) { return NULL; }
    
    return cmd_seq; 
}



/* Purpose: Convert a string to argv array array. */
char ***parse_cmd_seq(char *str)
{
    int i, j;

    static char *cmds[MAX_CMD_COUNT + 1];
    memset(cmds, '\0', sizeof(cmds));

    cmds[0] = str_strip(strtok(str, "|"));
    for (i = 1; i <= MAX_CMD_COUNT; ++i)
    {
        cmds[i] = str_strip(strtok(NULL, "|"));
        if (cmds[i] == NULL) { break; }
    }

    static char *argvs_array[MAX_CMD_COUNT + 1][MAX_ARG_COUNT + 1];
    static char **argvs[MAX_CMD_COUNT + 1];

    memset(argvs_array, '\0', sizeof(argvs_array));
    memset(argvs, '\0', sizeof(argvs));

    for (i = 0; cmds[i]; ++i)
    {
        argvs[i] = argvs_array[i];

        argvs[i][0] = strtok(cmds[i], " \t\n\r");
        for (j = 1; j <= MAX_ARG_COUNT; ++j)
        {
            argvs[i][j] = strtok(NULL, " \t\n\r");
            if (argvs[i][j] == NULL) { break; }
        }
    }

    return argvs;
}



/* Purpose: Strip to whitespace at the front and end of the string. */
char *str_strip(char *str)
{
    if (!str) { return str; }

    while (isspace(*str)) { ++str; }

    char *last = str;
    while (*last != '\0') { ++last; }
    last--;

    while (isspace(*last)) { *last-- = '\0'; }

    return str;
}



/* Purpose: Count the fequency of a character in the string. */
int str_char_count(char const *str, char c)
{
    int count = 0;

    if (str)
    {
        while (*str != '\0')
        {
            if (*str++ == c) { count++; }
        }
    }

    return count;
}
