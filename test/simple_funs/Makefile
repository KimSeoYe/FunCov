all:
	clang -fsanitize=address -c trace-pc-guard.c
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o

clean:
	rm example *.o

