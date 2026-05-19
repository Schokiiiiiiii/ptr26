/******************************************************************************
 * Copyright (C) 2026 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: xbuf.c
 *
 * Utilisation des Proxy
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
#include <string.h>
#include <sched.h>

#include <evl/thread.h>
#include <evl/clock.h>
#include <evl/proxy.h>
#include <evl/timer.h>

#define RT_PERIOD_TIME_MS 100000000   /* 100 ms */
#define START_DELAY_NS     50000000   /*  50 ms */
#define NB_MEASURES       100000000   /* 100'000'000 */

typedef struct thread_arg {
    int proxy_fd;
} thread_arg_t;

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

void *thread_sensors(void *arg)
{
    thread_arg_t *thread_arg = arg;
    int proxy_fd = thread_arg->proxy_fd;

    int ret;
    int tfd;
    int tmfd;
    cpu_set_t cpuset;
    struct itimerspec value;
    struct timespec now;
    __u64 ticks;
    char line[128];

    double t = 0.0;
    double s1;
    double s2;

    // Set CPU preference to 1
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);

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
    value.it_interval.tv_nsec = RT_PERIOD_TIME_MS;

    // Set timer
    if (evl_set_timer(tmfd, &value, NULL)) {
        evl_printf("evl_set_timer() failed\n");
        goto free_tmfd;
    }

    // Loop over each measures after each tick and write inside gpio
    for (int i = 0; i < NB_MEASURES; ++i) {
        ret = oob_read(tmfd, &ticks, sizeof(ticks));
        if (ret < 0) {
            evl_printf("oob_read() failed\n");
            break;
        }

        s1 = 2.0 * sin(t);
        s2 = 5.0 * sin(t + 0.8);

        ret = snprintf(line, sizeof(line), "%f %f\n", s1, s2);
        if (ret < 0 || ret >= sizeof(line)) {
            evl_printf("snprintf() failed\n");
            break;
        }

        ret = oob_write(proxy_fd, line, strlen(line));
        if (ret < 0) {
            evl_printf("oob_write() failed\n");
            break;
        }

        t += 0.1;
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

int main(int argc, char *argv[])
{
    int log_fd;
    pthread_t thread;           // thread for measures
    thread_arg_t thread_arg;
    pthread_attr_t attr;        // attributes for thread
    struct sched_param param;   // parameters for scheduling
    cpu_set_t cpuset;           // cpu preferences
    int ret;

    if (argc < 2) {
        printf("usage: %s logfile\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open log file linux side
    log_fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (log_fd < 0) {
        perror("open() failed\n");
        return EXIT_FAILURE;
    }

    // Set affinity on CPU 0
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    ret = sched_setaffinity(getpid(), sizeof(cpuset), &cpuset);
    if (ret) {
        fprintf(stderr, "sched_setaffinity() failed\n");
        return EXIT_FAILURE;
    }

    // PROXY CREATION
    // Create proxy for log file
    thread_arg.proxy_fd  = evl_create_proxy(
        log_fd,                               /* fichier cible Linux */
        4096,                                 /* taille du buffer interne */
        0,                                    /* granularité des écritures */
        EVL_CLONE_OUTPUT | EVL_CLONE_PRIVATE, /* proxy privé en sortie */
        "sensors_proxy"                       /* nom du proxy */
    );

    if (thread_arg.proxy_fd < 0) {
        fprintf(stderr, "evl_create_proxy() failed\n");
        ret = thread_arg.proxy_fd;
        goto close_log;
    }

    // RT TASK CREATION
    // Thread attributes
    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_init() failed\n");
        goto close_proxy;;
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

    // Set schedule priority as sth standard
    param.sched_priority = 10;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret != 0) {
        fprintf(stderr, "pthread_attr_setschedparam() failed\n");
        goto destroy_attr;
    }

    // Create periodic thread
    ret = pthread_create(&thread, &attr, thread_sensors, &thread_arg);
    if (ret != 0) {
        fprintf(stderr, "pthread_create() failed\n");
        goto destroy_attr;
    }

    // WAIT COMPLETION OF EVL THREAD
    // Wait for end
    ret = pthread_join(thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "pthread_join() failed\n");
        goto destroy_attr;
    }

    ret = EXIT_SUCCESS;

destroy_attr:
    pthread_attr_destroy(&attr);
close_proxy:
    close(thread_arg.proxy_fd);
close_log:
    close(log_fd);
    return ret;
}