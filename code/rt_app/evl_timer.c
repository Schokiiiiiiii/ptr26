/******************************************************************************
 * Copyright (C) 2024 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: evl_timer.c
 *
 * PTR labo3, step 3 "Thread Xenomai" adapted to EVL
 *
 *   Note: It works on Raspberry PI 4 and DE1SoC
 *
 *****************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <evl/evl.h>
#include <evl/timer.h>

#include <time.h>
#include <signal.h>
#include <sys/mman.h>

#include "../de1soc_io/de1soc_io.h"

#define PERIOD_NS       10000000
#define START_DELAY_NS  50000000

int nbMeasures;
struct timespec *measures;
volatile sig_atomic_t gpio_value = 0;

void *evl_thread(void *arg __attribute_maybe_unused__) {
    int tfd;                    // thread
    int tmfd;                   // timer
    cpu_set_t cpuset;           // cpu preferences
    struct sched_param param;   // parameters for scheduling
    struct itimerspec value;    // timer specifications
    struct timespec now;        // current time
    __u64 ticks;                // ticks for timer

    // Set CPU preference to 1
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);

    // Set parameters
    memset(&param, 0, sizeof(param));
    param.sched_priority = 10;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param)) {
        fprintf(stderr, "pthread_setschedparam() failed\n");
        return NULL;
    }

    // Attach self to evl
    tfd = evl_attach_self("evl-timer:pid%d\n", getpid());
    if (tfd < 0) {
        evl_printf("evl-timer:evl_attach_self() failed\n");
        return NULL;
    }

    // Create new timer
    tmfd = evl_new_timer(EVL_CLOCK_MONOTONIC);
    if (tmfd < 0) {
        evl_printf("evl-timer:evl_new_timer() failed\n");
        goto free_tfd;
    }

    // Set memory to null
    memset(&value, 0, sizeof(value));

    // Check current time
    evl_read_clock(EVL_CLOCK_MONOTONIC, &now);

    // Put start time from now with a delay
    value.it_value = now;
    value.it_value.tv_sec += START_DELAY_NS / 1000000000;
    value.it_value.tv_nsec += START_DELAY_NS % 1000000000;

    // Check for possible overflow in nsec
    if (value.it_value.tv_nsec >= 1000000000) {
        value.it_value.tv_sec++;
        value.it_value.tv_nsec -= 1000000000;
    }

    // Fix intervals every PERIOD_NS
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = PERIOD_NS;

    // Set timer
    if (evl_set_timer(tmfd, &value, NULL)) {
        evl_printf("evl_set_timer() failed\n");
        goto free_tmfd;
    }

    // Loop over each measures after each tick and write inside gpio
    for (int i = 0; i < nbMeasures + 1; ++i) {
        oob_read(tmfd, &ticks, sizeof(ticks));

        evl_read_clock(EVL_CLOCK_MONOTONIC, &measures[i]);

        gpio_value = gpio_value ? 0x0 : 0x00000002;
        write_gpio_val(0, REG_LOW, gpio_value);
    }

    // Set values back to default
    memset(&value, 0, sizeof(value));
    evl_set_timer(tmfd, &value, NULL);

free_tmfd:
    close(tmfd);
free_tfd:
    close(tfd);

    return NULL;
}

int main(int argc, char *argv[]) {
	pthread_t thread;           // thread for measures
    int ret;                    // return values for failure

    // Lock all current pages and future pages for this process
    mlockall(MCL_CURRENT|MCL_FUTURE);

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

    // Create periodic thread
    ret = pthread_create(&thread, NULL, (void *)evl_thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_create() failed: %s\n", strerror(ret));
        goto free_measures;;
    }

    // Wait for end
    ret = pthread_join(thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", strerror(ret));
        goto free_measures;
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

    return EXIT_SUCCESS;

free_measures:
    free(measures);
clear_io:
    write_gpio_val(0, REG_LOW, 0x0);
    clear_de1soc_io();

    return ret;
}
