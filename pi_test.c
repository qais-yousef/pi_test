#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

static bool low_prio_started;
static volatile int counter;

void busy_loop(void)
{
	while (1)
		counter++;
}

void *low_prio_thread(void *data)
{
	fprintf(stdout, "Low Prio thread started\n");
	low_prio_started = true;
	busy_loop();
	return NULL;
}

void *high_prio_thread(void *data)
{
	while (!low_prio_thread);
	fprintf(stdout, "High Prio thread started\n");
	busy_loop();
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t lp, hp, busy;
	pthread_attr_t lp_attr;
	struct sched_param lp_param = { 0 };
	int err;

	err = pthread_attr_init(&lp_attr);
	if (err) {
		fprintf(stderr, "Failed to init pthread_attr: %d\n", err);
		return err;
	}
	err = pthread_attr_setschedparam(&lp_attr, &lp_param);
	if (err) {
		fprintf(stderr, "Failed to set prio: %d\n", err);
		return err;
	}
	pthread_attr_setinheritsched(&lp_attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&lp, &lp_attr, low_prio_thread, NULL);
	pthread_create(&hp, NULL, high_prio_thread, NULL);

	busy_loop();

	pthread_attr_destroy(&lp_attr);
	pthread_join(lp, NULL);
	pthread_join(hp, NULL);

	return 0;
}
