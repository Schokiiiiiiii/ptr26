#define _GNU_SOURCE

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//#define N_SEC 5

int main(void) {

    const long start_time = time(NULL);
    int current_cpu = sched_getcpu();

    const struct tm *lt = localtime(&start_time);
    printf("%d:%d:%d : cpu started on %d\n", lt->tm_hour, lt->tm_min, lt->tm_sec, current_cpu);

    //for (time_t current_time = time(NULL) ; current_time < (start_time + N_SEC) ; current_time = time(NULL))  {
    while (1) {
        time_t current_time = time(NULL);
        const int new_cpu = sched_getcpu();
        if (current_cpu != new_cpu) {
            lt = localtime(&current_time);
            printf("%d:%d:%d : cpu changed to %d\n", lt->tm_hour, lt->tm_min, lt->tm_sec, new_cpu);
            current_cpu = new_cpu;
        }
    }

    return EXIT_SUCCESS;
}
