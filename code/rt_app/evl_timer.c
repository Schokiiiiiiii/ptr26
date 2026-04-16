/******************************************************************************
 * Copyright (C) 2024 HEIG-VD, REDS Institute
 *
 * authors: Jean-Pierre Miceli <jean-pierre.miceli@heig-vd.ch>
 * file: evl_timer.c
 *
 * PTR labo3, step 3 "Thread Xenomai" adapted to EVL
 *
 *   Note: It works on Raspberry PI 4 and DE1SoC
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

#include "../de1soc_io/de1soc_io.h"

void evl_thread(void *arg)
{
	/*...*/
}

int main(int argc, char *argv[])
{
	/*...*/

	return 0;
}
