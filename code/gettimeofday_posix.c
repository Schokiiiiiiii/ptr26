#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define NB_MESURES 30

static void test_clock(clockid_t clk_id, const char *clk_name) {

    struct timespec res;
    struct timespec t[NB_MESURES];
    int i;

    // read theoretical resolution
    if (clock_getres(clk_id, &res) != 0) {
        fprintf(stderr, "clock_getres(%s) failed: %s\n", clk_name, strerror(errno));
        return;
    }

    // print header
    printf("=== %s ===\n", clk_name);
    printf("Theoretical resolution : %ld.%09ld s\n", res.tv_sec, res.tv_nsec);

    // get times
    for (i = 0; i < NB_MESURES; ++i) {
        if (clock_gettime(clk_id, &t[i]) != 0) {
            fprintf(stderr, "clock_gettime(%s) failed: %s\n", clk_name, strerror(errno));
            return;
        }
    }

    // print measures
    for (i = 0; i < NB_MESURES; ++i) {
        printf("%2d : %ld.%09ld\n", i, t[i].tv_sec, t[i].tv_nsec);
    }

    printf("\n");
}

int main(int argc, char **argv) {

    // realtime system wide clock
    test_clock(CLOCK_REALTIME, "CLOCK_REALTIME");

    // non settable system wide clock
    test_clock(CLOCK_MONOTONIC, "CLOCK_MONOTONIC");

    return EXIT_SUCCESS;
}