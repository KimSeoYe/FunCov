all: 
	clang -fsanitize=address -c trace-pc-guard.c
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o math_file_fuzzer math_file_fuzzer.c ../mpc.c trace-pc-guard.o
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o math_fuzzer math_fuzzer.c ../mpc.c trace-pc-guard.o

clean:
	rm *.o math_file_fuzzer math_fuzzer
