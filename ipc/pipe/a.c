//
// Created by Gary Lai on 10/24/17.
//

/*
 * Author: Hari Caushik
 * Date Last Modified: 2/18/13
 * Description: sorts and prints all words in the file with their frequency
*/
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_PIPES 1000 		/* The maximum number of pipes */
#define BUF_SIZE 100 		/* maximum size of buffer used to get words */

static int npipes = 1;		/* number of sorters, set by cmd line arg */

int isAllEmpty(FILE *pipes[MAX_PIPES], int npipes);  /* checks if all pipes
							are empty */
int findFirst(char **wlist, char *first, int npipes); /* finds alphabetically
							first word from a
							list */
/* signal handler for parent */
static void phandler(int sig)
{
    int i;
    for (i = 0; i < npipes; i++)
    {
        wait(NULL);
    }
    _exit(EXIT_FAILURE);
}
/* signal handler for child */
static void chandler(int sig)
{
    _exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int sortfd[MAX_PIPES][2];	/* pipefds that deal with sorter */
    int suppfd[MAX_PIPES][2];	/* pipefds that deal with suppressor */
    char buf[BUF_SIZE];		/* buffer used to get words */
    FILE *insort[MAX_PIPES];	/* file stream that writes words to
						sorter */
    FILE *outsupp[MAX_PIPES];	/* file stream that reads from sorter
						*/

    /* merger variables */
    char **wlist;			/* word list containing words from
						each sorter to merge */
    char *last;			/* used in merger to store last word
						printed */
    char *next;			/* used to store the next word to be
						printed */
    int pipeno;			/* the pipe from which a word came
						from */
    int dupw;			/* keeps track of word frequency */


    struct sigaction sa;		/* for parent */
    struct sigaction sa_chd;	/* for child */

    npipes = atoi(argv[1]);

    if(npipes > MAX_PIPES)
    {
        printf("The limit is %d pipes.\n", MAX_PIPES);
        exit(EXIT_FAILURE);
    }
    /* install parent signal handler */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = phandler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("sigaction failed\n");
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
        perror("sigaction failed\n");
    if (sigaction(SIGHUP, &sa, NULL) == -1)
        perror("sigaction failed\n");

    /* create sorter pipes */
    int i;
    for (i = 0; i < npipes; i++)
    {
        /* create pipes for each sorter */
        if (pipe(sortfd[i]) == -1)
            perror("Could not open pipe\n");
        /* create pipes for suppressor - sorter communication */
        if (pipe(suppfd[i]) == -1)
            perror("Could not open pipe\n");
        /* create child processes */
        switch(fork()) {
            case -1:
                perror("fork\n");
            case 0: /*child*/
                /* child signal handler */
                sigemptyset(&sa_chd.sa_mask);
                sa_chd.sa_flags = 0;
                sa_chd.sa_handler = chandler;
                if (sigaction(SIGINT, &sa_chd, NULL) == -1)
                    perror("sigaction failed\n");
                if (sigaction(SIGQUIT, &sa_chd, NULL) == -1)
                    perror("sigaction failed\n");
                if (sigaction(SIGHUP, &sa_chd, NULL) == -1)
                    perror("sigaction failed\n");

                /* close unused write end  of sorter pipe*/
                if (close(sortfd[i][1]) == -1)
                    perror("could not close write end\n");
                /* close unused read end of merger pipe */
                if (close(suppfd[i][0]) == -1)
                    perror("could not close read end\n");
                /* sort reads from STDIN */
                if (sortfd[i][0] != STDIN_FILENO)
                {
                    if (dup2(sortfd[i][0], STDIN_FILENO) == -1)
                        perror("Could not connect to stdin\n");
                    if (close(sortfd[i][0]) == -1)
                        perror("could not close pipe\n");
                }
                /* sort writes to STDOUT */
                if (suppfd[i][1] != STDOUT_FILENO)
                {
                    if (dup2(suppfd[i][1], STDOUT_FILENO) == -1)
                        perror("Could not connect to stdout\n");
                    if (close(suppfd[i][1]) == -1)
                        perror("could not close pipe\n");
                }

                execl("/bin/sort", "/bin/sort", (char *)NULL);
            default: /*parent*/
                /* close unused read end of sorter pipe */
                if (close(sortfd[i][0]) == -1)
                    perror("Could not close pipe\n");
                /* close unused write end of merger */
                if (close(suppfd[i][1]) == -1)
                    perror("Could not open pipe\n");
                outsupp[i] = fdopen(suppfd[i][0],"r");	/* read from
								sorter */
                insort[i] = fdopen(sortfd[i][1], "w"); /* write to
								merger */
                break;
        }
    }

    /* write words into pipe */
    pipeno = 0;
    while (scanf("%[A-Za-z]", buf) != EOF)
    {
        scanf("%*c");	/* read blank or newline */
        for (i = 0; i < strlen(buf); i++)
            buf[i] = tolower(buf[i]);
        fputs(buf, insort[pipeno]);
        fputc('\n', insort[pipeno]);
        if (pipeno < npipes - 1)
            pipeno++;	/* get ready to write to next sorter */
        else
            pipeno = 0;	/* start over */
    }

    /* close each of sort streams */
    for (i = 0; i < npipes; i++)
    {
        if(fclose(insort[i]))
            perror("error closing file stream\n");
    }

    /* merger */
    wlist = malloc(npipes * sizeof(char *));
    for (i = 0; i < npipes; i++)
    {
        wlist[i] = malloc(BUF_SIZE);
    }
    last = malloc(BUF_SIZE);
    next = malloc(BUF_SIZE);

    /* get first word from all pipes */
    if (!isAllEmpty(outsupp, npipes))
    {
        for (i = 0; i < npipes; i++)
        {
            if(!feof(outsupp[i]) && !ferror(outsupp[i])
               && fgets(buf, BUF_SIZE, outsupp[i]) != NULL)
            {
                strcpy(wlist[i], buf);
            }
            else
            {
                wlist[i] = '\0'; /* pipe is empty */
            }
        }
    }

    /* write alphabetically first word */
    last[0] = '\0';
    dupw = 0;
    while (!isAllEmpty(outsupp, npipes))
    {

        pipeno = findFirst(wlist, next, npipes);
        if (strcmp(next, last) == 0)
        {
            dupw++;		/* duplicate */
        }
        else
        {
            if (!dupw)	/* first word gotten */
            {
                dupw++;
            }
            else		/* new word */
            {
                printf("%d %s", dupw, last);
                dupw = 1;
            }
            strcpy(last, next);
        }
        if (!feof(outsupp[pipeno]) && !ferror(outsupp[pipeno])
            && fgets(buf, BUF_SIZE, outsupp[pipeno]) != NULL)
        {
            strcpy(wlist[pipeno], buf); /* refill list with next
							word from same pipe */
        }
        else
            wlist[pipeno] = '\0';	/* pipe is empty */
    }

    /* close all sorter reader file streams */
    for (i = 0; i < npipes; i++)
    {
        if(fclose(outsupp[i]))
            perror("error closing file stream\n");
    }
    /* reap up all children sorters*/
    for (i = 0; i < npipes; i++)
    {
        wait(NULL);
    }

    free(wlist);
    free(next);
    free(last);
    return 0;
}

/*
 * checks if all pipes are empty
 * Inputs:
 * 	pipes[MAX_PIPES]: the sort pipes
 * 	npipes: number of sorter pipes
 * Outputs:
 * 	1 if empty, 0 if not empty
*/
int isAllEmpty(FILE *pipes[MAX_PIPES], int npipes)
{
    int isEmpty = 0;
    int i;
    for (i = 0; i < npipes; i++)
    {
        if (!feof(pipes[i]) && !ferror(pipes[i]))
        {
            return 0;
        }
    }
    return 1;
}

/*
 * finds alphabetically first word from a list of words
 * Inputs:
 * 	wlist: list of words
 * 	first: buffer that will contain the alphabetically first word from the
 * 		list
 *	npipes: the number of sorters
 * Outputs:
 * 	pipe from which the word was gotten
*/
int findFirst(char **wlist, char *first, int npipes)
{
    int pipeno;

    /* start with the first word available */
    int i;
    for (i = 0; i < npipes; i++)
    {
        if (wlist[i] != '\0')
        {
            pipeno = i;
            strcpy(first, wlist[i]);
            break;
        }
    }
    /* compare with all other words in list */
    for (i = pipeno + 1; i < npipes; i++)
    {
        if (wlist[i] == '\0')
            continue;
        /* word in list comes first alphabetically compared to the
             word stored in the first buffer */
        if (strcmp(first, wlist[i]) > 0)
        {
            strcpy(first, wlist[i]);
            pipeno = i;
        }
    }
    return pipeno;
}
