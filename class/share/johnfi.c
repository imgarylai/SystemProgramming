/* program to illustrate sending half of fifo on System V
argument tells how many numbers to send to mary */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

main(int argc, char *argv[]) {
  int fifofd; /*file descriptor for fifo */
  int j;      /*loop counter */
  int n;      /* total number of items to send */

  if (argc != 2) {
    printf("Usage: %s num\n", argv[0]);
    exit(3);
  }

  n = atoi(argv[1]);

  /* make the fifo - ignore error if it exists */

  if ((mkfifo("/tmp/john", 0666) == -1) && errno != EEXIST) {
    perror("mkfifo");
    exit(1);
  }

  /* open fifo with O_NONBLOCK so we won't block forever */
  while ((fifofd = open("/tmp/john", O_WRONLY | O_NONBLOCK)) == -1) {
    if (errno == ENXIO) {
      printf("WHERE is that Mary\n");
      sleep(20);
    } else {
      perror("fifo open");
      exit(2);
    }
  }

  /* John will generate numbers and Mary will add them*/
  for (j = 1; j <= n; j++)
    while (write(fifofd, &j, sizeof(int)) == -1) {
      printf("I seem to always wait for Mary\n");
      sleep(20);
    }
  close(fifofd); /* close the output fifo*/
  printf("I wrote %d number to Mary\n", n);
}
