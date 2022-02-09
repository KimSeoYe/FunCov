# FunCov
Function Coverage Tool using [LLVM Sanitizer Coverage](https://clang.llvm.org/docs/SanitizerCoverage.html)

This is a tool to examine which function of the target program was called by the prepared input files. It measures function pair coverage (**callee funciton, caller funcition, called location**). It has been created in the process of studying sanitizer coverage to add change-aware fuzzing components to [AFL++](https://github.com/AFLplusplus/AFLplusplus).


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

You need to use the provided `trace-pc-guard.c` and `shm_coverage.c` for your target project. Build your project using `clang` w/ `src/trace-pc-guard.c` and `src/shm_coverage.c`, especially using `-rdynamic`, `-fsanitize=address` and `-fsanitize-coverage=func,trace-pc-guard` options.

```bash
cp trace-pc-guard.c test/simple_example/
cd test/simple_example/
clang -fsanitize=address -c ../../src/trace-pc-guard.c ../../src/shm_coverage.c
clang -g -rdynamic -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o shm_coverage.o
```

### 2. Execute `funcov`

If the output directory does not exist, funcov will make it.

```bash
# usage: ./funcov -i [input_dir] -o [output_dir] -x [executable_binary] ... 

# required
# -i : input directory path
# -o : output directory path
# -x : executable binary path
    
# optional
# @@ : input type - file path as an argument (default: stdin)

../../funcov -i inputs/ -o out/ -x example

```

### 3. Result directory

* **bitmaps/** : It has information on covered functions for each input file as bitmaps. You can turn on this option manually (by defining `SAVE_MAP` at funcov.c).
* **covered_funs/** : It has information on covered functions for each input file as .csv files. (callee function name, caller function name, PC value where the callee function called)
* **err/** : standard error results for each input file
* **out/** : standard output results for each input file
* **per_cov_log.csv** : The number of covered functions and exit code for each input file
* **trace_cov_log.csv** : The log of accumulated function coverage. You can use this file to draw the following coverage graph.
<img width="498" alt="image" src="https://user-images.githubusercontent.com/47961698/150488384-3e753db3-8684-454d-b7c8-6b750b3cb4e2.png">


### 4. Result screen

```bash
FUNCOV ARGS
* EXECUTABLE BINARY: /home/kimseoye/git/FunCov/test/simple_example/example
* INPUT TYPE: stdin
* OUTPUT DIR PATH: /home/kimseoye/git/FunCov/tmp
* INPUT DIR PATH: /home/kimseoye/git/FunCov/test/simple_example/inputs
* INPUT FILE CNT: 3
* INPUT FILES
  [0] /home/kimseoye/git/FunCov/test/simple_example/inputs/negative_example
  [1] /home/kimseoye/git/FunCov/test/simple_example/inputs/zero_example
  [2] /home/kimseoye/git/FunCov/test/simple_example/inputs/positive_example

# cov: function pair coverage, acc_cov: accumulated coverage
RUN
* [0] /home/kimseoye/git/FunCov/test/simple_example/inputs/negative_example: cov=1, acc_cov=1
* [1] /home/kimseoye/git/FunCov/test/simple_example/inputs/zero_example: cov=1, acc_cov=2
* [2] /home/kimseoye/git/FunCov/test/simple_example/inputs/positive_example: cov=1, acc_cov=3

WRITE /home/kimseoye/git/FunCov/tmp/per_cov_log.csv for...
* [0] /home/kimseoye/git/FunCov/test/simple_example/inputs/negative_example
* [1] /home/kimseoye/git/FunCov/test/simple_example/inputs/zero_example
* [2] /home/kimseoye/git/FunCov/test/simple_example/inputs/positive_example

WRITE /home/kimseoye/git/FunCov/tmp/trace_cov_log.csv for...
* [0] /home/kimseoye/git/FunCov/test/simple_example/inputs/negative_example
* [1] /home/kimseoye/git/FunCov/test/simple_example/inputs/zero_example
* [2] /home/kimseoye/git/FunCov/test/simple_example/inputs/positive_example

WRITE /home/kimseoye/git/FunCov/tmp/covered_funs for...
* [0] /home/kimseoye/git/FunCov/test/simple_example/inputs/negative_example
* [1] /home/kimseoye/git/FunCov/test/simple_example/inputs/zero_example
* [2] /home/kimseoye/git/FunCov/test/simple_example/inputs/positive_example

RESULT SUMMARY
* INITIAL COVERAGE: 1
* TOTAL COVERAGE: 3
* LOG SAVED IN /home/kimseoye/git/FunCov/tmp/per_cov_log.csv
* ACCUMULATED LOG SAVED IN /home/kimseoye/git/FunCov/tmp/trace_cov_log.csv
* COVERED FUNTIONS PER INPUT SAVED IN /home/kimseoye/git/FunCov/tmp/covered_funs

WE ARE DONE!
```


## To Do

* Make a wrapper program of `clang` for preparation step
* Make a tool which reads result bitmaps and interpretes them