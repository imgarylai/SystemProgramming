#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define MIN_LENGTH_OF_WORD 4
#define MAX_LENGTH_OF_WORD 40
#define MAX_SORT_PROC 10

int main(int argc, char* argv[]) {

    int sort_pipes[2];
    int sup_pipes[2];
    FILE* sort_write;
    FILE* sup_readers;

    if (pipe(sort_pipes) == -1) {
        exit(EXIT_FAILURE);
    };
    if (pipe(sup_pipes) == -1) {
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

    // Create Process
    pid_t sort_id = fork();
    if (sort_id < 0) {
        exit(EXIT_FAILURE);
    } else if (sort_id == 0) {
        printf("sort child\n");
        // read from std
        dup2(sort_pipes[0], STDIN_FILENO);
        dup2(sup_pipes[1], STDOUT_FILENO);
        close(sort_pipes[0]);
        close(sort_pipes[1]);
        close(sup_pipes[0]);
        close(sup_pipes[1]);
        execlp("sort", "sort", (char*) NULL);
        perror("");
    }

    pid_t sup_id = fork();
    if (sup_id < 0) {
        exit(EXIT_FAILURE);
    } else if (sup_id == 0) {
        printf("sup child\n");
        close(sort_pipes[0]);
        close(sort_pipes[1]);
        close(sup_pipes[1]);
        sup_readers = fdopen(sup_pipes[0], "r");
        int c;
        while ((c = fgetc(sup_readers)) != EOF) {
            putchar(c);
        }
        close(sup_pipes[0]);
        fclose(sup_readers);
    }

    close(sort_pipes[0]);
    close(sort_pipes[1]);
    close(sup_pipes[0]);
    close(sup_pipes[1]);
    wait(NULL);
    wait(NULL);

    return EXIT_SUCCESS;
}
