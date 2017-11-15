/* program to illustrate use of utime and ctime.
        program prints out old access and modification
        dates, and then retards and advances them by 10
        minutes respectively */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

int main(int argc, char **argv) {
  struct stat statbuf;
  struct utimbuf newt;

  if (argc != 2) {
    printf("Usage: chdate file\n");
    exit(1);
  }

  /* get stat info */
  if (stat(argv[1], &statbuf) == -1) {
    perror("stat");
    exit(1);
  }

  printf("Last accessed: %s\n", ctime(&statbuf.st_atime));
  printf("Last modified: %s\n", ctime(&statbuf.st_mtime));
  printf("Last changed: %s\n", ctime(&statbuf.st_ctime));

  newt.actime = statbuf.st_atime - 600;
  newt.modtime = statbuf.st_mtime + 600;

  /* now go set new times */

  if (utime(argv[1], &newt) == -1) {
    perror("utime");
    exit(1);
  }

  exit(0);
}
