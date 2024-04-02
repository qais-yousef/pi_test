pi_test: pi_test.c
	cc $^ -o $@

all: pi_test

clean:
	rm -f *.o pi_test
