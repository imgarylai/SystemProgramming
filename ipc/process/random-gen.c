#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

enum { RANDOM_NUMBER_NEED_COUNT = 10 };

int main()
{
    srand(time(NULL));

    int i;
    for (i = 0; i < RANDOM_NUMBER_NEED_COUNT; ++i)
    {
        sleep(1); /* Wait 1 second.  Simulate the complex process of
                     generating the safer random number. */

        int randnum = rand() % 100;
        write(STDOUT_FILENO, &randnum, sizeof(int));
    }

    return EXIT_SUCCESS;
}
