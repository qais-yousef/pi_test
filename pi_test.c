/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2024 Qais Yousef */

#define _GNU_SOURCE
#include <argp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

static volatile bool low_prio_started;
static volatile int counter;
static pthread_mutexattr_t mutex_pi;
static pthread_mutex_t mutex;

static int lp_nice = 10;
static int hp_nice = 0;
static int pin_cpu = 0;

#define barrier()	asm volatile ("" ::: "memory")


/* handle arguments */
const char *argp_program_version = "pi_test v0.1";
const char *argp_program_bug_address = "<qyousef@layalina.io>";

static char doc[] =
"Test PTHREAD_PRIO_INHERIT for fair tasks by spawning a thread at low nice value blocking another thread at higher nice value.";

enum pi_test_opts_flags {
	OPT_DUMMY_START = 0x80,

	OPT_LP_NICE,
	OPT_HP_NICE,

	OPT_AFFINE_CPU,
};

static const struct argp_option options[] = {
	{ "lp-nice", OPT_LP_NICE, "NICE", 0, "Nice value of low priority thread to run at. Default 10." },
	{ "hp-nice", OPT_HP_NICE, "NICE", 0, "Nice value of high priority thread to run at. Default 0." },
	{ "affine-cpu", OPT_AFFINE_CPU, "NICE", 0, "CPU to affine the tasks to. Default 0." },
	{ 0 },
};

static error_t parse_arg(int key, char *arg, struct argp_state *state)
{
	char *end_ptr;

	switch (key) {
	case OPT_LP_NICE:
		errno = 0;
		lp_nice = strtol(arg, &end_ptr, 0);
		if (errno != 0) {
			perror("Unsupported lp_nice value\n");
			return errno;
		}
		if (end_ptr == arg) {
			fprintf(stderr, "lp_nice: no digits were found\n");
			argp_usage(state);
			return -EINVAL;
		}
		break;
	case OPT_HP_NICE:
		errno = 0;
		hp_nice = strtol(arg, &end_ptr, 0);
		if (errno != 0) {
			perror("Unsupported hp_nice value\n");
			return errno;
		}
		if (end_ptr == arg) {
			fprintf(stderr, "hp_nice: no digits were found\n");
			argp_usage(state);
			return -EINVAL;
		}
		break;
	case OPT_AFFINE_CPU:
		errno = 0;
		pin_cpu = strtol(arg, &end_ptr, 0);
		if (errno != 0) {
			perror("Unsupported pin_cpu value\n");
			return errno;
		}
		if (end_ptr == arg) {
			fprintf(stderr, "pin_cpu: no digits were found\n");
			argp_usage(state);
			return -EINVAL;
		}
		break;
	case ARGP_KEY_ARG:
		argp_usage(state);
		break;
	case ARGP_KEY_END:
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

const struct argp argp = {
	.options = options,
	.parser = parse_arg,
	.doc = doc,
};

void busy_loop(void)
{
	while (1)
		counter++;
}

int get_nice(void)
{
	int nice;

	nice = getpriority(PRIO_PROCESS, 0);
	if (nice == -1) {
		fprintf(stderr, "Failed to get nice\n");
		return 0;
	}

	return nice;
}

void set_nice(int nice)
{
	int err;

	err = setpriority(PRIO_PROCESS, 0, nice);
	if (err)
		fprintf(stderr, "Failed to change nice to %d\n", nice);
}

void *low_prio_thread(void *data)
{
	pthread_setname_np(pthread_self(), "pi_test_low");
	set_nice(lp_nice);
	fprintf(stdout, "Low Prio thread started, nice: %d\n", get_nice());
	pthread_mutex_lock(&mutex);
	barrier();
	low_prio_started = true;
	busy_loop();
	return NULL;
}

void *high_prio_thread(void *data)
{
	pthread_setname_np(pthread_self(), "pi_test_high");
	set_nice(hp_nice);
	fprintf(stdout, "High Prio thread started, nice: %d\n", get_nice());
	while (!low_prio_thread);
	barrier();
	pthread_mutex_lock(&mutex);
	fprintf(stderr, "Error: High Prio thread should never run.\n");
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t lp, hp, busy;
	cpu_set_t cpuset;
	int err;

	err = argp_parse(&argp, argc, argv, 0, NULL, NULL);
	if (err)
		return err;

	err = pthread_mutexattr_init(&mutex_pi);
	if (err)
		fprintf(stderr, "Failed to init mutexattr\n");
	err = pthread_mutexattr_setprotocol(&mutex_pi, PTHREAD_PRIO_INHERIT);
	if (err)
		fprintf(stderr, "Failed to set mutex protocol\n");
	err = pthread_mutex_init(&mutex, &mutex_pi);
	if (err)
		fprintf(stderr, "Failed to init mutex\n");

	CPU_ZERO(&cpuset);
	CPU_SET(pin_cpu, &cpuset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);

	pthread_create(&lp, NULL, low_prio_thread, NULL);
	pthread_create(&hp, NULL, high_prio_thread, NULL);

	busy_loop();

	pthread_join(lp, NULL);
	pthread_join(hp, NULL);

	return 0;
}
