/* Program to Illustrate Async I/O with non-synchromized writes */
/* argv[1] file to write */
/* artv[2] number of writes */

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NUMBYTES 1024 * 10240
#define PAUSE 1024 * 1024

char buff[NUMBYTES];

void main(int argc, char *argv[]) {

  int fd;
  struct aiocb cb;
  int count, numwrites, i, j;

  count = 0;

  numwrites = atoi(argv[2]);

  /*Use Non-Synchronized I/O */
  if ((fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0640)) == -1) {
    perror("Open failed: ");
    exit(1);
  }

  /* Fill in AIO Control Block */

  cb.aio_fildes = fd;
  cb.aio_buf = buff;
  cb.aio_nbytes = NUMBYTES;
  cb.aio_sigevent.sigev_notify = SIGEV_NONE;
  /* No signals */
  cb.aio_offset = 0;
  cb.aio_reqprio = 0;

  for (i = 0; i < numwrites; i++) {
    aio_write(&cb);
    while (aio_error(&cb)) { /* Waste Time - other work goes here */
      j = PAUSE;
      while (j)
        j--;
      count++;
    }
    lseek(fd, 0, SEEK_SET);
  }
  printf("count=%d\n", count);
}
