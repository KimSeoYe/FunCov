all: 
	export ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-6.0/bin/llvm-symbolizer
	clang -fsanitize=address -c trace-pc-guard.c
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o

clean:
	rm *.o example