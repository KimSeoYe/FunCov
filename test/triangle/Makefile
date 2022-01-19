all:
	clang -fsanitize=address -c trace-pc-guard.c
	clang -g -fsanitize=address -fsanitize-coverage=trace-pc-guard -o triangle triangle.c trace-pc-guard.o

clean:
	rm triangle *.o


