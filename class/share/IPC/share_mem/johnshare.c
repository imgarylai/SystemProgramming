/* Program to illustrate sharing memory on System V */
/* John will create a vector of numbers in shared memory */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#define KEY (key_t)12347 /*key for shared memory segment */

main() {

  int sid;    /* segment id of shared memory segment */
  int *array; /* pointer to shared array, no storage yet*/
  int j;      /*loop counter */
  /*
  char *shmat();
  */

  /* create shared segment if necessary */
  if ((sid = shmget(KEY, 100 * sizeof(int), IPC_CREAT | 0660)) == -1) {
    perror("shmget");
    exit(1);
  }

  /* map it into our address space*/

  if ((array = ((int *)shmat(sid, 0, 0))) == (int *)-1) {
    perror("shmat");
    exit(2);
  }

  /* Now fill it up */

  for (j = 0; j <= 100; j++)
    array[j] = j;
}
