/* Program to illustrate sharing memory on System V */
/* John will create a vector of numbers in shared memory */
/* John will use a semaphore, */
/* Also uses a semaphore to lock the region from Mary
	until John is done with it. Note this is only
	a one way lock.*/

#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#define KEY (key_t)12346	/*key for shared memory segment */

main() {

	int sid;	/* segment id of shared memory segment */
	int *array;	/* pointer to shared array, no storage yet*/
	int j;		/*loop counter */
	int semid;	/* semaphore id */
	struct sembuf sb;	/* semaphore buffer */

		
	/* create shared segment if necessary */
	if ((sid=shmget(KEY,100*sizeof (int),IPC_CREAT |0660))== -1) {
		perror("shmget");
		exit(1);
	}

	/* map it into our address space*/
	if ((array=((int *) shmat(sid,0,0)))== (int *) -1) {
		perror("shmat");
		exit(2);
	}

	/* Now fill it up */
	for (j=0;j<=100;j++) array[j]=j;

	/* get semaphore id*/
	if ((semid=semget(KEY,1 ,IPC_CREAT |0660))== -1) {
		perror("semget");
		exit(1);
	}
	
	sb.sem_op = 1;  /* set up for a unlock operation*/
	sb.sem_num = 0;  
	sb.sem_flg = 0;  

	if (semop(semid, &sb, 1) == -1) { /* should not block */
		perror("sem unlock");
		exit(1);
	}
}

