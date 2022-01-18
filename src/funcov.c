#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "../include/funcov.h"

static cov_arg_t covarg ;

/**
 * usage: ./funcov -i [input_dir] -x [executable_binary] [...]
 * 
 * required
 * -i : input directory path
 * -x : executable binary path
 * 
 * optional
 * @@ : input type - file as an argument
*/

void
print_covarg ()
{
    printf("\n============================== FUNCOV ARGS ==============================\n") ;
    printf("* EXECUTABLE BINARY: %s\n", covarg.binary_path) ;
    printf("* INPUT DIR PATH: %s\n", covarg.input_dir_path) ;
    printf("* INPUT TYPE: ") ;
    switch (covarg.input_type) {
    case STDIN:
        printf("standard input\n") ;
        break;
    case ARG_FILENAME:
        printf("file as an argument\n") ;
        break;
    }
    printf("=========================================================================\n\n") ;
}

void
get_cmd_args (int argc, char * argv[])
{
    int i_flag = 0, x_flag = 0 ;
    int arg_cnt = 1 ;

    int opt ;
    while ((opt = getopt(argc, argv, "i:x:")) != -1) {
        switch(opt) {
        case 'i':
            if (realpath(optarg, covarg.input_dir_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid input directory") ;
                exit(1) ;
            }
            i_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        case 'x':
            if (realpath(optarg, covarg.binary_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid executable binary") ;
                exit(1) ;
            }
            x_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        }
    }

    if (!i_flag || !x_flag) {
        perror("usage: ./funcov -i [input_dir] -x [executable_binary] [...]\n\nrequired\n-i : input directory path\n-x : executable binary path\n\noptional\n@@ : input type - file as an argument") ;
        exit(1) ;
    }

    if (argc > 5 && strcmp(argv[arg_cnt], "@@") == 0) {
        covarg.input_type = ARG_FILENAME ;
        arg_cnt++ ;
    } 
    else {
        covarg.input_type = STDIN ;
    }

    print_covarg() ;
}

int
main (int argc, char * argv[])
{
    get_cmd_args(argc, argv) ;
    

    return 0 ;
}