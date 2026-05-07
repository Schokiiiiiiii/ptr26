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

int main(int argc, char *argv[])
{
    pthread_t worker1_thr;
    pthread_t worker2_thr;
    rt_params_t worker1_params;
    rt_params_t worker2_params;
    struct evl_flags flg;
    int fd;
    int ret;

    (void)argc;
    (void)argv;

    /* Creation of the event flags */

    /* Creation of the worker & watchdog threads */

    /* Wait the completion of the evl thread */

}
