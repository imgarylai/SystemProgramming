// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING
// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - GARY LAI
//
// Created by Gary Lai on 10/21/17.
//


// sort sort.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ARGS_NUM 3
#define MIN_LENGTH_OF_WORD 4
#define MAX_LENGTH_OF_WORD 40
#define MAX_SORTING_PROC 10

void usage(int argc);
/* Purpose: Create child process and redirect io.
 * (Only available for execute_cmd_seq().)
 */
void creat_proc(int fd_in, int fd_out,
                int pipes_count, int pipes_fd[][2]);

int main(int argc, char* argv[]) {
    // Check Arguments
    usage(argc);
    int P;
    // Parse Commands
    char *in_file = argv[1];
    int sort_proc_num = atoi(argv[2]) >= MAX_SORTING_PROC ? MAX_SORTING_PROC : atoi(argv[2]);
    int pipe_num = sort_proc_num;

    // Init pipes
    int sort_pipes[pipe_num][2];
    int suppressor_pipes[pipe_num][2];
    for(P = 0; P < pipe_num; ++P) {
        if ((pipe(sort_pipes[P]) == -1) || (pipe(suppressor_pipes[P]) == -1)) {
            fprintf(stderr, "Error: Unable to create pipe. (%d)\n", P);
            exit(EXIT_FAILURE);
        } else {
            printf("Pipe %d is created\n", P);
        }
    }

    for (P = 0; P < pipe_num; ++P) {
        int fd_in = sort_pipes[P][0];
        int fd_out = sort_pipes[P][1];
        pid_t pid;
        if ((pid = fork()) < 0) {
            perror("Error: Unable to fork.");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            // child process
            // Redirection
            printf("Child");
            close(fd_in);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
            /* 載入可執行檔，我直接把 argv[0] 當成 executable name */
            if (execlp("ls", "ls", "-al", (char *)0) == -1) {
                perror("");
                exit(EXIT_FAILURE);
            }

            /* NEVER REACH */
            exit(EXIT_FAILURE);
        } else {
            // parent process
//            int status;
//            wait(&status); /* 等程式執行完畢 */
        }
    }

    // Close pipes
    for (P = 0; P < pipe_num; ++P) {
        close(sort_pipes[P][0]);
        close(sort_pipes[P][1]);
        close(suppressor_pipes[P][0]);
        close(suppressor_pipes[P][1]);
    }


//    FILE *in_stream;
//    int c;
//    if ((in_stream = fopen(in_file, "r")) == 0) {
//        perror("");
//        exit(EXIT_FAILURE);
//    } else {
//        int found_word = 0;
//        while ((c = fgetc(in_stream)) != EOF){
//            if(isalpha(c)) {
//                found_word = 1;
//                c = tolower(c);
//                putchar(c);
//            } else {
//                if(found_word) {
//                    putchar('\n');
//                    found_word = 0;
//                }
//            }
//        }
//    }
//    fclose(in_stream);

    return 0;
}

void usage(int argc) {
    if (argc != ARGS_NUM) {
        fprintf(stderr, "Error: Incorrect Arguments.\n");
        fprintf(stderr, "Usage: $ uniqify STDIN_FILE NUM_OF_SORTING_PROCESS\n");
        exit(EXIT_FAILURE);
    }
};
