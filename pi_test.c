#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/resource.h>

static bool low_prio_started;
static volatile int counter;

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
	low_prio_started = true;
	busy_loop();
	return NULL;
}

void *high_prio_thread(void *data)
{
	while (!low_prio_thread);
	fprintf(stdout, "High Prio thread started, nice: %d\n", get_nice());
	busy_loop();
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t lp, hp, busy;

	pthread_create(&lp, NULL, low_prio_thread, NULL);
	pthread_create(&hp, NULL, high_prio_thread, NULL);

	busy_loop();

	pthread_join(lp, NULL);
	pthread_join(hp, NULL);

	return 0;
}
