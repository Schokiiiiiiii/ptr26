#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>

#define EXECUTION_TIME    10 /* In seconds */

/* Barrier variable */
pthread_barrier_t barr;
time_t start_time;

void *f_thread(void *arg) {
    long int *iterations = (long int *)arg;
    int count = 0;

    // wait on other threads
    pthread_barrier_wait(&barr);

    // count for the time
    while (time(NULL) < (start_time + EXECUTION_TIME)) {
        ++count;
    }

    // put count back to iterations
    *iterations = count;

    return NULL;
}

int main(int argc, char **argv) {

    int i;
    int nb_threads = 0;
    long int total_iterations = 0;
    int min_prio = 0;
    int max_prio = 0;

    /* Parse input */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s NB_THREADS\n", argv[0]);
        return EXIT_FAILURE;
    }
    nb_threads = strtoimax(argv[1], (char **)NULL, 10);
    if (nb_threads <= 0) {
        fprintf(stderr, "NB_THREADS must be > 0 (actual: %d)\n", nb_threads);
        return EXIT_FAILURE;
    }

    /* ... */
    // no

    /* Why did we add these following lines? */

    /* Set thread attributes necessary to use priorities */
    struct sched_param schedparam;
    pthread_attr_t thread_attr;
    schedparam.sched_priority = 1;
    pthread_attr_init(&thread_attr);
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    if (pthread_attr_setschedparam(&thread_attr, &schedparam) != 0) {
        fprintf(stderr, "Could not set scheduler parameters!\n");
        return EXIT_FAILURE;
    }


    /* Get minimal and maximal priority values */
    min_prio = sched_get_priority_min(SCHED_FIFO);
    max_prio = sched_get_priority_max(SCHED_FIFO);
    max_prio -= min_prio;

    printf("Min prio : %d\nMax prio : %d\n\n", min_prio, max_prio);

    /* Initialize barrier */
    if (pthread_barrier_init(&barr, NULL, nb_threads)) {
        fprintf(stderr, "Could not initialize barrier!\n");
        return EXIT_FAILURE;
    }

    // set start_time (need it to be the same for each threads so best place)
    start_time = time(NULL);

    /* Set priorities and create threads */
    pthread_t threads[nb_threads];
    int prio_value[nb_threads];
    long int nb_iterations[nb_threads];
    for (i = 0 ; i < nb_threads ; ++i) {
        // chose prio value
        prio_value[i] = (nb_threads == 1 ? min_prio : min_prio + i * (max_prio - min_prio) / (nb_threads - 1));

        // rebuild attributes
        schedparam.sched_priority = prio_value[i];
        pthread_attr_setschedparam(&thread_attr, &schedparam);

        // create thread
        if (pthread_create(&threads[i], &thread_attr, f_thread, &nb_iterations[i]) != 0) {
            printf("pthread_create() had an issue\n");
            return EXIT_FAILURE;
        }
    }

    /* Wait for the threads to complete and set the results */
    for (i = 0 ; i < nb_threads ; ++i) {
        pthread_join(threads[i], NULL);
        total_iterations += nb_iterations[i];
    }

    for (i = 0; i < nb_threads; ++i) {
        fprintf(stdout, "[%02d] %ld (%2.0f%%)\n",
                prio_value[i], nb_iterations[i],
                100.0 * nb_iterations[i] / total_iterations);
    }

    return EXIT_SUCCESS;
}
