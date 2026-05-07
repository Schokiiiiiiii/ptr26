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
#include <error.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <evl/evl.h>
#include <evl/timer.h>

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
    /* Make thread an evl thread */

    /* Create timer */

    while (true) {

        /* Wait for the next tick to be notified. */

        /* Read sample info - the actual time */

        /* Write the sample in the xbuf */

    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* ... */

    /* Create the evl thread */

    /* in-band processing */

    /* Wait the completion of the evl thread */

    return 0;
}
