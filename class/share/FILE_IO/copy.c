/* Program to copy a file, using a specified buffer size */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int fd1, fd2;    /* file descriptors */
  char buff[4096]; /* overkill on buffer size*/
  int size;        /* real buffer size */
  int iosize;      /* actual amount read */

  if ((fd1 = open(argv[1], 0)) == -1) {
    perror("open1");
    exit(1);
  }

  if ((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
    perror("open2");
    exit(1);
  }

  size = atoi(argv[3]);

  while ((iosize = read(fd1, buff, size)) > 0)
    write(fd2, buff, iosize);

  if (iosize == -1) {
    perror("read");
    exit(1);
  }
}
