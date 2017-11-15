#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  /* for perror*/
#include <stdlib.h> /* for exit */
#include <sys/stat.h>
#include <sys/types.h> /* for creat */
#include <unistd.h>    /* for close */

int main() {
  int fd;
  if ((fd = creat("abc", 0)) == -1) {
    perror("creat");
    exit(1);
  }
  close(fd);
}
