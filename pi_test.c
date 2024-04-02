#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/resource.h>

static volatile bool low_prio_started;
static volatile int counter;
static pthread_mutexattr_t mutex_pi;
static pthread_mutex_t mutex;

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
	set_nice(10);
	fprintf(stdout, "Low Prio thread started, nice: %d\n", get_nice());
	pthread_mutex_lock(&mutex);
	low_prio_started = true;
	busy_loop();
	return NULL;
}

void *high_prio_thread(void *data)
{
	fprintf(stdout, "High Prio thread started, nice: %d\n", get_nice());
	while (!low_prio_thread);
	pthread_mutex_lock(&mutex);
	fprintf(stderr, "Error: High Prio thread should never run.\n");
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t lp, hp, busy;
	int err;

	err = pthread_mutexattr_init(&mutex_pi);
	if (err)
		fprintf(stderr, "Failed to init mutexattr\n");
	err = pthread_mutexattr_setprotocol(&mutex_pi, PTHREAD_PRIO_INHERIT);
	if (err)
		fprintf(stderr, "Failed to set mutex protocol\n");
	err = pthread_mutex_init(&mutex, &mutex_pi);
	if (err)
		fprintf(stderr, "Failed to init mutex\n");

	pthread_create(&lp, NULL, low_prio_thread, NULL);
	pthread_create(&hp, NULL, high_prio_thread, NULL);

	busy_loop();

	pthread_join(lp, NULL);
	pthread_join(hp, NULL);

	return 0;
}
