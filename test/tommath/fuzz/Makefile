all:
	clang -fsanitize=address -c trace-pc-guard.c
	clang -fsanitize=address -fsanitize-coverage=func,trace-pc-guard tommath.a fuzz_tommath.c -o fuzz_tom trace-pc-guard.o

clean:
	rm *.o fuzz_tom