/******************************************************************************
 * Copyright (C) 2026 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: xbuf.c
 *
 * Utiliation des Proxy
 *
 *****************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <error.h>

#include <evl/thread.h>
#include <evl/clock.h>
#include <evl/proxy.h>
#include <evl/timer.h>

#define RT_PERIOD_TIME_MS 100000000   /* 100 ms */

typedef struct sample {
    double s1;
    double s2;
} sample_t;

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

int main(int argc, char *argv[])
{
    int log_fd;

    if (argc < 2) {
        printf("usage: %s logfile\n", argv[0]);
        return 1;
    }

    /* ouverture fichier log Linux */
    log_fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log_fd < 0) {
        perror("open");
        return 1;
    }

    /* proxy creation*/

    /* RT task creation */

    /* Wait the completion of the evl thread */

    return 0;
}