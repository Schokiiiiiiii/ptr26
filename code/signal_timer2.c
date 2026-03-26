#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

int nbMeasures;

/* ... */

int main (int argc, char *argv[])
{
    long usec;
    long nsec;

    /* ... */

    /* Test the parameters */
    if (argc != 3) {
        fprintf(stderr, "Wrong parameter(s)\n");
        return EXIT_FAILURE;
    }

    /* Set the measures and duration */
    nbMeasures = strtoimax(argv[1], (char **)NULL, 10);
    usec = (long)strtol(argv[2], (char **)NULL, 10);
    if (nbMeasures <= 0 || usec <= 0) {
        fprintf(stderr,
                "Both the number of measures and the time (in us) must be > 0\n");
        return EXIT_FAILURE;
    }

    nsec = usec * 1000;

    /* ... */

    return EXIT_SUCCESS;
}
