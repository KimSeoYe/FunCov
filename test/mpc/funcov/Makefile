all: 
	clang -fsanitize=address -c ../../../src/trace-pc-guard.c ../../../src/shm_coverage.c
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o math_file_fuzzer math_file_fuzzer.c ../mpc.c trace-pc-guard.o shm_coverage.o
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o math_fuzzer math_fuzzer.c ../mpc.c trace-pc-guard.o shm_coverage.o

clean:
	rm *.o math_file_fuzzer math_fuzzer
