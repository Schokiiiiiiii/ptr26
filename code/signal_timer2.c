#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

int nbMeasures;
volatile sig_atomic_t stopped = 0;
struct timespec *measures;

void handler_signal(int signum) {
    static int count = 0;

    if (count >= nbMeasures + 1) {
        stopped = 1;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &measures[count++]);
    }
}

int main (int argc, char *argv[]) {
    long usec;
    long nsec;
    struct sigevent event  = {0};
    struct itimerspec spec = {0};
    timer_t timerid;

    // Test parameters
    if (argc != 3) {
        fprintf(stderr, "Wrong parameter(s)\n./signal_timer2 <nb_measures> <usec>\n");
        return EXIT_FAILURE;
    }

    // Set the measures and duration
    nbMeasures = strtoimax(argv[1], (char **)NULL, 10);
    usec = (long)strtol(argv[2], (char **)NULL, 10);
    if (nbMeasures <= 0 || usec <= 0) {
        fprintf(stderr,
                "Both the number of measures and the time (in us) must be > 0\n");
        return EXIT_FAILURE;
    }

    // Allocate measures
    measures = calloc(nbMeasures + 1, sizeof(struct timespec));

    // Register signal
    signal(SIGRTMIN, handler_signal);
    event.sigev_notify = SIGEV_SIGNAL;
    event.sigev_signo  = SIGRTMIN;

    // Convert micro-seconds to nanoseconds
    nsec = usec * 1000;

    // Configure timer
    spec.it_interval.tv_sec  = nsec / 1000000000;
    spec.it_interval.tv_nsec = nsec % 1000000000;
    spec.it_value = spec.it_interval;

    // Allocate timer
    if (timer_create(CLOCK_REALTIME, &event, &timerid) == -1) {
        fprintf(stderr, "timer_create() had a problem\n");
        return EXIT_FAILURE;
    }

    // Launch timer
    if (timer_settime(timerid, 0, &spec, NULL) == -1) {
        fprintf(stderr, "timer_settime() had a problem\n");
        return EXIT_FAILURE;
    }

    // Zzz
    while (!stopped) {
        pause();
    }

    for (int i = 0 ; i < nbMeasures ; ++i) {
        long difference = (measures[i+1].tv_sec  - measures[i].tv_sec) * 1000000000 +
                          (measures[i+1].tv_nsec - measures[i].tv_nsec);
        /*
        printf("%d : %ld\n", i, difference);
        */
        printf("%ld\n", difference);
    }

    // Delete timer
    timer_delete(timerid);

    return EXIT_SUCCESS;
}
