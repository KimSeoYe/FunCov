# FunCov
Function Coverage Tool using [LLVM Sanitizer Coverage](https://clang.llvm.org/docs/SanitizerCoverage.html)


## Build This Project
```bash
$ git clone https://github.com/FunCov.git
$ cd FunCov
$ make
```

## Quick Start Guide w/ a Simple Example

### Target project : test/simple_example/

It is just a very simple program to determine whether the input is positive, negative, or zero. Because we want to measure the function coverage, it uses 3 functions to print out the result.

Input files in `test/simple_example/inputs/` containing 10000, -10000, and 0 were prepared to measure coverage.


```c
#include <stdio.h>
#include <stdlib.h>

void
negative ()
{
	printf("the input is negative\n") ;
}

void
zero ()
{
	printf("the input is zero\n") ;
}

void
positive ()
{
	printf("the input is positive\n") ;
}

int
main ()
{
	int input ;
	scanf("%d", &input) ;
	
	if (input < 0) negative() ;
	else if (input > 0) positive() ;
	else zero() ;

	return 0 ;
}

```

### 1. Prepare your target program

1. Copy the provided `trace-pc-guard.c` into the directory of your target project.
2. Build your project using `clang` w/ `trace-pc-guard.c`, especially using `-fsanitize=address` and `-fsanitize-coverage=func,trace-pc-guard` options.

```bash
cp trace-pc-guard.c test/simple_example/
cd test/simple_example/
clang -fsanitize=address -c trace-pc-guard.c
clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o
```

### 2. Execute `funcov`

You need to properly set `ASAN_SYMBOLIZER_PATH` to successfully obtain the name of the function.<br>
If the output directory does not exist, funcov will make it.

```bash
# usage: ./funcov -i [input_dir] -o [output_dir] -x [executable_binary] ... 

# required
# -i : input directory path
# -o : output directory path
# -x : executable binary path
    
# optional
# @@ : input type - file path as an argument (default: stdin)

export ASAN_SYMBOLIZER_PATH=$(which llvm-symbolizer)
../../funcov -i inputs/ -o out/ -x example

```

### 3. Result directory

* **bitmaps/** : It has information on covered functions for each input file as bitmaps.
* **covered_funs/** : It has information on covered functions for each input file as .csv files.
* **err/** : standard error results for each input file
* **out/** : standard output results for each input file
* **per_cov_log.csv** : The number of covered functions and exit code for each input file
* **trace_cov_log.csv** : The log of accumulated function coverage

### 4. Result screen

```bash
FUNCOV ARGS
* EXECUTABLE BINARY: /home/git/FunCov/test/simple_example/example
* INPUT TYPE: stdin
* OUTPUT DIR PATH: /home/git/FunCov/test/simple_example/out
* INPUT DIR PATH: /home/git/FunCov/test/simple_example/inputs
* INPUT FILE CNT: 3
* INPUT FILES
    [0] /home/git/FunCov/test/simple_example/inputs/negative_example
    [1] /home/git/FunCov/test/simple_example/inputs/zero_example
    [2] /home/git/FunCov/test/simple_example/inputs/positive_example

# cov: the number of covered functions, acc_cov: accumulated function coverage
RUN 
* [0] /home/git/FunCov/test/simple_example/inputs/negative_example: cov=2, acc_cov=2
* [1] /home/git/FunCov/test/simple_example/inputs/zero_example: cov=2, acc_cov=3
* [2] /home/git/FunCov/test/simple_example/inputs/positive_example: cov=2, acc_cov=4

COVERED FUNCTIONS
* [0]  negative:/home/git/FunCov/test/simple_example/example.c:6
* [1]  zero:/home/git/FunCov/test/simple_example/example.c:12
* [2]  positive:/home/git/FunCov/test/simple_example/example.c:18
* [3]  main:/home/git/FunCov/test/simple_example/example.c:24

RESULT SUMMARY
* INITIAL COVERAGE: 2
* TOTAL COVERAGE: 4
* LOG SAVED IN /home/git/FunCov/test/simple_example/out/per_cov_log.csv
* ACCUMULATED LOG SAVED IN /home/git/FunCov/test/simple_example/out/trace_cov_log.csv
* FUNCTION COVERAGE BITMAPS SAVED IN /home/git/FunCov/test/simple_example/out/bitmaps
* COVERED FUNTIONS PER INPUT SAVED IN /home/git/FunCov/test/simple_example/out/covered_funs

WE ARE DONE!
```


## To Do

* Make a wrapper program of `clang` for preparation step
* Make a tool which reads result bitmaps and interpretes them