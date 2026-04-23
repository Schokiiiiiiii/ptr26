#define _GNU_SOURCE

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>
#include <sched.h>

#include "../../de1soc_io/de1soc_io.h"

int nbMeasures;
struct timespec *measures;
volatile sig_atomic_t gpio_value = 0;

void *thread_timer(void *arg __attribute_maybe_unused__) {
    // sleep is every 10ms
    const struct timespec time = {0, 10000000};

    // loop to get all measures
    for (int i = 0 ; i < nbMeasures + 1 ; ++i) {
        // get time
        clock_gettime(CLOCK_MONOTONIC, &measures[i]);

        // Toggle GPIO_0_D1
        gpio_value = gpio_value ? 0x0 : 0x00000002;
        write_gpio_val(0, REG_LOW, gpio_value);

        // sleep
        nanosleep(&time, NULL);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;           // thread for measures
    pthread_attr_t attr;        // attributes for thread
    struct sched_param param;   // parameters for scheduling
    cpu_set_t cpuset;           // cpu preferences
    int ret;                    // return values for failure

    // Test parameters
    if (argc != 2) {
        fprintf(stderr, "Wrong parameter(s)\n./pthread_time.c <nb_measures>\n");
        return EXIT_FAILURE;
    }

    // Set the measures and fixed duration = 10 ms
    nbMeasures = strtoimax(argv[1], (char **)NULL, 10);
    if (nbMeasures <= 0) {
        fprintf(stderr, "The number of measures must be > 0\n");
        return EXIT_FAILURE;
    }

    // Set affinity on CPU 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    ret = sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    if (ret != 0) {
        fprintf(stderr, "[ERROR] Set affinity failed\n");
        return ret;
    }

    // Init DE1-SoC IO
    ret = init_de1soc_io();
    if (ret < 0) {
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
        ret = EXIT_FAILURE;
        goto clear_io;
    }

    // Thread attributes
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_init() failed\n");
        goto free_measures;
    }

    // Explicit scheduling
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_setinheritsched() failed\n");
        goto destroy_attr;
    }

    // Use SCHED_FIFO
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_setschedpolicy() failed\n");
        goto destroy_attr;
    }

    // High priority
    memset(&param, 0, sizeof(param));
    param.sched_priority = sched_get_priority_max(SCHED_FIFO) - 10;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_setschedparam() failed\n");
        goto destroy_attr;
    }

    // Create periodic thread
    ret = pthread_create(&thread, &attr, thread_timer, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_create() failed: %s\n", strerror(ret));
        goto destroy_attr;
    }

    // Wait for end
    ret = pthread_join(thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", strerror(ret));
        goto destroy_attr;
    }

    // Print measures
    for (int i = 0; i < nbMeasures; ++i) {
        long difference =
            (measures[i + 1].tv_sec  - measures[i].tv_sec) * 1000000000L +
            (measures[i + 1].tv_nsec - measures[i].tv_nsec);
        printf("%ld\n", difference);
    }

    // Cleanup
    write_gpio_val(0, REG_LOW, 0x0);
    clear_de1soc_io();
    free(measures);
    pthread_attr_destroy(&attr);

    return EXIT_SUCCESS;

destroy_attr:
    pthread_attr_destroy(&attr);
free_measures:
    free(measures);
clear_io:
    write_gpio_val(0, REG_LOW, 0x0);
    clear_de1soc_io();

    return ret;
}

