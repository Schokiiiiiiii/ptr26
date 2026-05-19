/******************************************************************************
 * Copyright (C) 2026 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: flags.c
 *
 * Utiliation des flags
 *
 *****************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <error.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <evl/thread.h>
#include <evl/clock.h>
#include <evl/flags.h>
#include <evl/evl.h>

/* Worker threads maximum expected working time, in ms */
#define WORKER1_PROCESS_TIME_MAX   2000
#define WORKER2_PROCESS_TIME_MAX   2500

/* Worker threads working time in error case, in ms */
#define WORKER2_PROCESS_TIME_ERROR 4000

/* Add a jitter in the worker porcess time, in ms */
#define WORKER_PROCESS_TIME_JITTER  10

/* Occurrence of Worker processing error (process duration is to long) */
#define WORKER_PROCESS_TIME_ERR   5

/* Watchdog timeout, in ms */
#define WATCHDOG_TIMEOUT           3500

#define WORKER1_FLAG     (1 << 0)
#define WORKER2_FLAG     (1 << 1)
#define WATCHDOG1_FLAG   (1 << 2)
#define WATCHDOG2_FLAG   (1 << 3)
#define SYNC_FLAG        (1 << 4)

#define NB_CYCLES                  10

typedef struct {
    unsigned idx;
    int process_time;
    unsigned flag_mask;
    struct evl_flags *flgs;
} rt_params_t;

static void timespec_add_ns(struct timespec *r,
                     const struct timespec *t,
                 long long ns)
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

static int sleep_ms(long ms)
{
    struct timespec now;
    struct timespec wakeup;
    int ret;

    ret = evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
    if (ret)
        return ret;

    timespec_add_ns(&wakeup, &now, ms * 1000000LL);

    return evl_sleep_until(EVL_CLOCK_MONOTONIC, &wakeup);
}

static void set_cpu1(void)
{
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
}

void *worker_task(void *arg)
{
    rt_params_t *params = (rt_params_t *)arg;
    int evl_fd;
    int ret;

    /* Make thread an EVL thread */
    evl_fd = evl_attach_self("worker-%u", params->idx);
    if (evl_fd < 0) {
        fprintf(stderr, "evl_attach_self() failed\n");
        return NULL;
    }

    /* Run worker on CPU 1 */
    set_cpu1();

    /* Worker is ready */
    if (params->idx == 1)
        evl_post_flags(params->flgs, WATCHDOG1_FLAG);
    else
        evl_post_flags(params->flgs, WATCHDOG2_FLAG);

    /* Synchronization barrier */
    evl_wait_exact_flags(params->flgs, SYNC_FLAG);

    for (int i = 1; i <= NB_CYCLES; ++i) {
        int jitter;
        int duration;

        /* Simulated controlled jitter */
        jitter = ((i * 7 + params->idx * 3) %
                 (2 * WORKER_PROCESS_TIME_JITTER + 1))
                 - WORKER_PROCESS_TIME_JITTER;

        duration = params->process_time + jitter;

        /* Simulated error on worker 2 */
        if (params->idx == 2 && i == WORKER_PROCESS_TIME_ERR)
            duration = WORKER2_PROCESS_TIME_ERROR;

        evl_printf("Worker %u cycle %d, work time = %d ms\n",
                   params->idx, i, duration);

        ret = sleep_ms(duration);
        if (ret) {
            evl_printf("Worker %u sleep failed\n", params->idx);
            break;
        }

        /* Notify watchdog that this worker completed its cycle */
        evl_post_flags(params->flgs, params->flag_mask);
    }

    close(evl_fd);
    return NULL;
}

void *watchdog_task(void *arg)
{
    rt_params_t *params = (rt_params_t *)arg;
    int evl_fd;
    int ret;
    int state;
    struct timespec now;
    struct timespec timeout;

    /* Make thread an EVL thread */
    evl_fd = evl_attach_self("watchdog");
    if (evl_fd < 0) {
        fprintf(stderr, "evl_attach_self() failed\n");
        return NULL;
    }

    /* Run watchdog on CPU 1 */
    set_cpu1();

    /* Synchronization barrier */
    evl_wait_exact_flags(params->flgs, SYNC_FLAG);

    for (int i = 1; i <= NB_CYCLES; ++i) {

        evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
        timespec_add_ns(&timeout, &now, WATCHDOG_TIMEOUT * 1000000LL);

        /* Wait for both workers */
        ret = evl_timedwait_exact_flags(params->flgs,
                                        WORKER1_FLAG | WORKER2_FLAG,
                                        &timeout);

        if (ret == 0) {
            evl_printf("Watchdog cycle %d: OK\n", i);
        } else if (ret == -ETIMEDOUT) {
            evl_peek_flags(params->flgs, &state);

            evl_printf("Watchdog cycle %d: TIMEOUT\n", i);

            if (!(state & WORKER1_FLAG))
                evl_printf(" -> Worker 1 missing\n");

            if (!(state & WORKER2_FLAG))
                evl_printf(" -> Worker 2 missing\n");

            /* Reset pending flags for next cycle */
            evl_trywait_flags(params->flgs, &state);
        } else {
            evl_printf("Watchdog wait failed\n");
            break;
        }
    }

    close(evl_fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t worker1_thr;
    pthread_t worker2_thr;
    pthread_t watchdog_thr;

    rt_params_t worker1_params;
    rt_params_t worker2_params;
    rt_params_t watchdog_params;

    struct evl_flags flg;
    int fd;
    int ret;

    (void)argc;
    (void)argv;

    /* Prevent memory paging */
    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
    if (ret != 0) {
        fprintf(stderr, "mlockall() failed\n");
        return EXIT_FAILURE;
    }

    /* Initialize EVL library */
    ret = evl_init();
    if (ret) {
        fprintf(stderr, "evl_init() failed\n");
        return EXIT_FAILURE;
    }

    /* Creation of the event flags */
    fd = evl_new_flags(&flg, "watchdog_flags");
    if (fd < 0) {
        fprintf(stderr, "evl_new_flags() failed\n");
        return EXIT_FAILURE;
    }

    worker1_params.idx = 1;
    worker1_params.process_time = WORKER1_PROCESS_TIME_MAX;
    worker1_params.flag_mask = WORKER1_FLAG;
    worker1_params.flgs = &flg;

    worker2_params.idx = 2;
    worker2_params.process_time = WORKER2_PROCESS_TIME_MAX;
    worker2_params.flag_mask = WORKER2_FLAG;
    worker2_params.flgs = &flg;

    watchdog_params.idx = 0;
    watchdog_params.process_time = 0;
    watchdog_params.flag_mask = 0;
    watchdog_params.flgs = &flg;

    /* Creation of the worker & watchdog threads */
    ret = pthread_create(&worker1_thr, NULL, worker_task, &worker1_params);
    if (ret != 0) {
        fprintf(stderr, "pthread_create(worker1) failed: %s\n", strerror(ret));
        close(fd);
        return EXIT_FAILURE;
    }

    ret = pthread_create(&worker2_thr, NULL, worker_task, &worker2_params);
    if (ret != 0) {
        fprintf(stderr, "pthread_create(worker2) failed: %s\n", strerror(ret));
        close(fd);
        return EXIT_FAILURE;
    }

    ret = pthread_create(&watchdog_thr, NULL, watchdog_task, &watchdog_params);
    if (ret != 0) {
        fprintf(stderr, "pthread_create(watchdog) failed: %s\n", strerror(ret));
        close(fd);
        return EXIT_FAILURE;
    }

    /* Wait  all threads blocked on the barrier. */
    usleep(100000);
    evl_broadcast_flags(&flg, SYNC_FLAG);

    /* Wait the completion of the EVL threads */
    pthread_join(worker1_thr, NULL);
    pthread_join(worker2_thr, NULL);
    pthread_join(watchdog_thr, NULL);

    evl_close_flags(&flg);
    close(fd);

    return EXIT_SUCCESS;
}
