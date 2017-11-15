/* Program to illustrate message quees on System V */
/* John will send mary data in small messages */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#define KEY (key_t)12345 /*key for message queue */

int main() {

  int qid; /* message queue id */
  int j;   /*loop counter */
  struct {
    long type;
    int data;
  } my_msg;

  /* create queue if necessary */
  if ((qid = msgget(KEY, IPC_CREAT | 0660)) == -1) {
    perror("msgget");
    exit(1);
  }

  /* Now send the numbers */

  for (j = 1; j <= 100; j++) {
    my_msg.type = 1 + (j % 2);
    my_msg.data = j;
    msgsnd(qid, &my_msg, sizeof(my_msg.data), 0);
  }

  /* send terminating messages */
  my_msg.type = 1;
  my_msg.data = -1;
  msgsnd(qid, &my_msg, sizeof(my_msg.data), 0);
  my_msg.type = 2;
  msgsnd(qid, &my_msg, sizeof(my_msg.data), 0);
}
