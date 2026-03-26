#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N_SEC 5
#define OP 1

int main(void) {

    const long start_time = time(NULL);

    int nb_iterations = 0;
    float a = 1000, b = 13;
    while (time(NULL) < (start_time + N_SEC)) {
#if OP
        a = a / b;
#endif // OP
        ++nb_iterations;
    }

    printf("PID%d There has been %d iterations in %d seconds!\n", getpid(), nb_iterations, (int) N_SEC);

    return EXIT_SUCCESS;
}
