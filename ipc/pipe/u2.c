#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main(void){
    int pipes[2];
    pid_t pid;
    FILE *stream;

    int stat;
    if(pipe(pipes) == -1)
        printf("could not create pipe\n");

    switch(fork()){
        case -1:
            fprintf(stderr, "error forking\n");
            break;
        case 0:
            dup2(pipes[0], STDIN_FILENO);

            pid = getpid();
            printf("in child, pid=%d\n", pid);

            if(close(pipes[1]) == -1)
                fprintf(stderr,"err closing write end pid=%d\n", pid);

            if(close(pipes[0]) == -1)
                fprintf(stderr,"err closing write end pid=%d\n", pid);

            execl("/bin/sort", "sort",  (char*) NULL);
            exit(EXIT_FAILURE);
            break;
        default:
            stream = fdopen(pipes[1], "w");
            pid = getpid();
            printf("in parent, pid=%d\n", pid);

            if (stream == NULL)
                fprintf(stderr, "could not create file streami\n");

            if(close(pipes[0]) == -1)
                printf("err closing read end pid=%d\n");

            fputs("bob\n",stream);
            fputs("cat\n",stream);
            fputs("ace\n",stream);
            fputs("dog\n",stream);

            if(fclose(stream) == EOF)
                fprintf(stderr, "error while closing stream\n");

            wait(&stat);
            break;
    }
    return 0;
}