/******************************************************************************
 * Copyright (C) 2026 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: xbuf.c
 *
 * Utiliation des Cross-buffer
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
#include <sys/mman.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#include <evl/evl.h>
#include <evl/timer.h>

#define PERIOD_NS 10000000L /* 10 ms */
#define XBUF_SIZE (64 * sizeof(sample_t))

int nbMeasures;

typedef struct {
    struct timespec time;
} sample_t;

typedef struct {
    int xbuf_fd;
} rt_params_t;

static void timespec_add_ns(struct timespec *__restrict r,
                     const struct timespec *__restrict t,
                 long ns)
{
    long s, rem;

    s = ns / 1000000000;
    rem = ns - s * 1000000000;
    r->tv_sec = t->tv_sec + s;
    r->tv_nsec = t->tv_nsec + rem;
    if (r->tv_nsec >= 1000000000) {
            r->tv_sec++;
            r->tv_nsec -= 1000000000;
    }
}

void *rt_task(void *arg)
{
    rt_params_t *params = (rt_params_t *)arg;

    int ret;
    int evl_fd;
    int timer_fd;
    uint64_t ticks;

    struct timespec now;
    struct itimerspec timer_spec;
    sample_t sample;
    cpu_set_t cpuset;

    /* Make thread an evl thread */
    evl_fd = evl_attach_self("rt_task");
    if (evl_fd < 0) {
        fprintf(stderr, "evl_attach_self() failed\n");
        return NULL;
    }

    /* Run the EVL task on CPU 1 (isolated CPU) */
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    ret = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    if (ret != 0) {
        fprintf(stderr, "sched_setaffinity() failed\n");
        close(evl_fd);
        return NULL;
    }

    /* Create timer */
    timer_fd = evl_new_timer(EVL_CLOCK_MONOTONIC);
    if (timer_fd < 0) {
        fprintf(stderr, "evl_new_timer() failed\n");
        close(evl_fd);
        return NULL;
    }

    /* Configure a 10 ms periodic timer */
    ret = evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
    if (ret != 0) {
        fprintf(stderr, "evl_read_clock() failed\n");
        close(timer_fd);
        close(evl_fd);
        return NULL;
    }

    memset(&timer_spec, 0, sizeof(timer_spec));
    timespec_add_ns(&timer_spec.it_value, &now, PERIOD_NS);
    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = PERIOD_NS;

    ret = evl_set_timer(timer_fd, &timer_spec, NULL);
    if (ret != 0) {
        fprintf(stderr, "evl_set_timer() failed\n");
        close(timer_fd);
        close(evl_fd);
        return NULL;
    }

    for (int i = 0; i < nbMeasures; ++i) {

        /* Wait for the next tick to be notified */
        ret = oob_read(timer_fd, &ticks, sizeof(ticks));
        if (ret < 0) {
            fprintf(stderr, "oob_read() failed\n");
            break;
        }

        /* Read sample info - the actual time */
        ret = evl_read_clock(EVL_CLOCK_MONOTONIC, &sample.time);
        if (ret != 0) {
            fprintf(stderr, "evl_read_clock() failed\n");
            break;
        }

        /* Write the sample in the xbuf */
        ret = oob_write(params->xbuf_fd, &sample, sizeof(sample));
        if (ret < 0) {
            fprintf(stderr, "oob_write() failed\n");
            break;
        }
    }

    close(timer_fd);
    close(evl_fd);

    return NULL;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    pthread_t thread;
    int ret;

    rt_params_t params;
    sample_t sample;
    struct timespec current_time;

    long previous_current_ns = 0;
    long previous_sample_ns = 0;

    /* Test parameters */
    if (argc != 2) {
        fprintf(stderr, "Wrong parameter(s)\n./xbuf <nb_measures>\n");
        return EXIT_FAILURE;
    }

    /* Get nbMeasures */
    nbMeasures = strtoimax(argv[1], (char **)NULL, 10);
    if (nbMeasures <= 0) {
        fprintf(stderr, "The number of measures must be > 0\n");
        return EXIT_FAILURE;
    }

    /* Prevent memory paging for real-time execution */
    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    if (ret != 0) {
        fprintf(stderr, "mlockall() failed\n");
        return EXIT_FAILURE;
    }

    /* Create a cross-buffer */
    params.xbuf_fd = evl_new_xbuf(XBUF_SIZE, "cross_buffer");
    if (params.xbuf_fd < 0) {
        fprintf(stderr, "evl_new_xbuf() failed\n");
        return EXIT_FAILURE;
    }

    /* Create the evl thread */
    ret = pthread_create(&thread, NULL, rt_task, &params);
    if (ret != 0) {
        fprintf(stderr, "pthread_create() failed: %s\n", strerror(ret));
        close(params.xbuf_fd);
        return EXIT_FAILURE;
    }

    /* in-band processing */
    for (int i = 0; i < nbMeasures; ++i) {

        /* Read data from the cross-buffer */
        ret = read(params.xbuf_fd, &sample, sizeof(sample));
        if (ret < 0) {
            fprintf(stderr, "read() failed\n");
            break;
        }

        /* Read current Linux time */
        ret = clock_gettime(CLOCK_MONOTONIC, &current_time);
        if (ret != 0) {
            fprintf(stderr, "clock_gettime() failed\n");
            break;
        }

        long current_ns =
            current_time.tv_sec * 1000000000L +
            current_time.tv_nsec;

        long sample_ns =
            sample.time.tv_sec * 1000000000L +
            sample.time.tv_nsec;

        printf("Current: %ld ns | Delta current: %ld ns | "
               "Xbuf: %ld ns | Delta xbuf: %ld ns\n",
               current_ns,
               previous_current_ns ? current_ns - previous_current_ns : 0,
               sample_ns,
               previous_sample_ns ? sample_ns - previous_sample_ns : 0);

        previous_current_ns = current_ns;
        previous_sample_ns = sample_ns;
    }

    /* Wait the completion of the evl thread */
    ret = pthread_join(thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join() failed: %s\n", strerror(ret));
        close(params.xbuf_fd);
        return EXIT_FAILURE;
    }

    close(params.xbuf_fd);

    return EXIT_SUCCESS;
}
