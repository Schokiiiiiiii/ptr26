#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NB_MESURES 30

int main (int argc, char **argv)
{
    struct timeval tv[NB_MESURES];
    int i;

    /* ... */

    return EXIT_SUCCESS;
}
