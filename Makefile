CC=gcc

pi_test: pi_test.c
	$(CROSS_COMPILE)$(CC) -O2 -g $^ -o $@ -lpthread -static

all: pi_test

clean:
	rm -f *.o pi_test
