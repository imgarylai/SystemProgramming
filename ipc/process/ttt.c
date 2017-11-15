#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    if (execlp("ls", "ls", (char *)0) == -1) {
        perror("");
        exit(EXIT_FAILURE);
    }
    return 0;
}