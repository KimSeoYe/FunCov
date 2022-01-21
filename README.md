# FunCov
Function Coverage Tool using Sanitizer Coverage


## Build This Project
```bash
git clone https://github.com/FunCov.git
cd FunCov
make
```

## Quick Start Guide a Simple Example

1. Prepare your target program

    1. Copy the provided `trace-pc-guard.c` into the directory of your target project.
    2. Build your project using `clang` w/ `trace-pc-guard.c`, especially using `-fsanitize-coverage=func,trace-pc-guard` option.
    ```bash
    cp trace-pc-guard.c test/simple_example/
    cd test/simple_example/
    clang -fsanitize=address -c trace-pc-guard.c
	clang -g -fsanitize=address -fsanitize-coverage=func,trace-pc-guard -o example example.c trace-pc-guard.o
    ```

2. Execute `funcov`
    ```bash
    # usage: ./funcov -i [input_dir] -o [output_dir] -x [executable_binary] ... 
    
    # required
    # -i : input directory path
    # -o : output directory path
    # -x : executable binary path
     
    # optional
    # @@ : input type - file as an argument

    ../../funcov -i inputs/ -o out/ -x example

    ```

3. Results
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

    RUN
    * [0] /home/git/FunCov/test/simple_example/inputs/negative_example: cov=2, acc_cov=2
    * [1] /home/git/FunCov/test/simple_example/inputs/zero_example: cov=2, acc_cov=3
    * [2] /home/git/FunCov/test/simple_example/inputs/positive_example: cov=2, acc_cov=4

    COVERED FUNCTIONS
    * [0]  negative:/home/git/FunCov/test/simple_example/example.c:6
    * [1]  zero:/home/git/FunCov/test/simple_example/example.c:12
    * [2]  positive:/home/git/FunCov/test/simple_example/example.c:18
    * [3]  main:/home/git/FunCov/test/simple_example/example.c:24

    RESULTS
    * INITIAL COVERAGE: 2
    * TOTAL COVERAGE: 4
    * LOG SAVED IN /home/git/FunCov/test/simple_example/out/per_cov_log.csv
    * ACCUMULATED LOG SAVED IN /home/git/FunCov/test/simple_example/out/trace_cov_log.csv
    * FUNCTION COVERAGE BITMAPS SAVED IN /home/git/FunCov/test/simple_example/out/bitmaps
    * COVERED FUNTIONS PER INPUT SAVED IN /home/git/FunCov/test/simple_example/out/covered_funs

    WE ARE DONE!
    ```