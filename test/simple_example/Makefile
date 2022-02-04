all:
	clang -fsanitize=address -c ../../src/trace-pc-guard.c ../../src/shm_coverage.c
	clang -g -rdynamic -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o shm_coverage.o

clean:
	rm *.o example