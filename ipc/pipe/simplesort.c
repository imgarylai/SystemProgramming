#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

    int sort_pipes[2];
    FILE* sort_write;

    if (pipe(sort_pipes) == -1) {
        exit(EXIT_FAILURE);
    };

    sort_write = fdopen(sort_pipes[1], "w");

    int c;
    int found_word = 0;
    while ((c = fgetc(stdin)) != EOF) {
        if(isalpha(c)) {
            found_word = 1;
            c = tolower(c);
            fputc(c, sort_write);
        } else {
            if(found_word) {
                fputc('\n', sort_write);
                found_word = 0;
            }
        }
    }

    fclose(sort_write);

    pid_t sort_id = fork();
    if (sort_id < 0) {
        exit(EXIT_FAILURE);
    } else if (sort_id == 0) {
        dup2(sort_pipes[0], STDIN_FILENO);
        close(sort_pipes[0]);
        close(sort_pipes[1]);
        execlp("sort", "sort", (char*) NULL);
        perror("");
        _exit(EXIT_SUCCESS);
    }

    close(sort_pipes[0]);
    close(sort_pipes[1]);
    wait(NULL);
}
