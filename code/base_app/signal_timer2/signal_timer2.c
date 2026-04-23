#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include <sched.h>
#include <stdbool.h>

#include "../../de1soc_io/de1soc_io.h"

int nbMeasures;
volatile sig_atomic_t stopped = 0;
struct timespec *measures;
volatile sig_atomic_t gpio_value = 0;

void handler_signal(int signum __attribute_maybe_unused__) {
    static int count = 0;

    if (count >= nbMeasures + 1) {
        stopped = 1;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &measures[count++]);

        // Toggle GPIO_0_D1
        gpio_value = gpio_value ? 0x0 : 0x00000002;
        write_gpio_val(0, REG_LOW, gpio_value);
    }
}

int main(int argc, char *argv[]) {
    long usec;
    long nsec;
    struct sigevent event  = {0};
    struct itimerspec spec = {0};
    timer_t timerid;
    cpu_set_t cpuset;
    int ret;

    // Test parameters
    if (argc != 2) {
        fprintf(stderr, "Wrong parameter(s)\n./signal_timer2 <nb_measures>\n");
        return EXIT_FAILURE;
    }

    // Set the measures and fixed duration = 10 ms
    nbMeasures = strtoimax(argv[1], (char **)NULL, 10);
    usec = 10000;

    if (nbMeasures <= 0) {
        fprintf(stderr, "The number of measures must be > 0\n");
        return EXIT_FAILURE;
    }

    /*
    // Set affinity on CPU 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    ret = sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    if (ret) {
        fprintf(stderr, "[ERROR] Set affinity failed\n");
        return EXIT_FAILURE;
    }
    */

    // Init DE1-SoC IO
    if (init_de1soc_io() < 0) {
        fprintf(stderr, "init_de1soc_io() failed\n");
        return EXIT_FAILURE;
    }

    // Enable GPIO_0_D1
    write_gpio_en(0, REG_LOW, 0x00000002);
    write_gpio_val(0, REG_LOW, 0x0);

    // Allocate measures
    measures = calloc(nbMeasures + 1, sizeof(struct timespec));
    if (measures == NULL) {
        fprintf(stderr, "calloc() failed\n");
        clear_de1soc_io();
        return EXIT_FAILURE;
    }

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
        free(measures);
        clear_de1soc_io();
        return EXIT_FAILURE;
    }

    // Launch timer
    if (timer_settime(timerid, 0, &spec, NULL) == -1) {
        fprintf(stderr, "timer_settime() had a problem\n");
        timer_delete(timerid);
        free(measures);
        clear_de1soc_io();
        return EXIT_FAILURE;
    }

    // Wait until enough measures have been collected
    while (!stopped) {
        pause();
    }

    for (int i = 0; i < nbMeasures; ++i) {
        long difference =
            (measures[i + 1].tv_sec  - measures[i].tv_sec) * 1000000000L +
            (measures[i + 1].tv_nsec - measures[i].tv_nsec);
        printf("%ld\n", difference);
    }

    // Cleanup
    write_gpio_val(0, REG_LOW, 0x0);
    timer_delete(timerid);
    clear_de1soc_io();
    free(measures);

    return EXIT_SUCCESS;
}