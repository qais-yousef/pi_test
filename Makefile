pi_test: pi_test.c
	cc -O2 -g $^ -o $@ -lpthread

all: pi_test

clean:
	rm -f *.o pi_test
