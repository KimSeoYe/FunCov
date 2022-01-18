#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <dirent.h>
#include "../include/funcov.h"

#define INPUT_CNT_UNIT 512

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
    printf("\nFUNCOV ARGS\n") ;
    printf("* EXECUTABLE BINARY: %s\n", covarg.binary_path) ;
    printf("* INPUT TYPE: ") ;
    switch (covarg.input_type) {
    case STDIN:
        printf("standard input\n") ;
        break;
    case ARG_FILENAME:
        printf("file as an argument\n") ;
        break;
    }
    printf("* INPUT DIR PATH: %s\n", covarg.input_dir_path) ;
    printf("* INPUT FILE CNT: %d\n", covarg.input_file_cnt) ;
    printf("* INPUT FILES\n") ;
    for (int i = 0; i < covarg.input_file_cnt; i++) {
        printf("  [%d] %s\n", i, covarg.input_files[i].file_path) ;
    }
    printf("\n") ;
}

int
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
                return -1 ;
            }
            i_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        case 'x':
            if (realpath(optarg, covarg.binary_path) == 0x0) {
                perror("get_cmd_args: realpath: Invalid executable binary") ;
                return -1 ;
            }
            x_flag = 1 ;
            arg_cnt += 2 ;
            break ;
        }
    }

    if (!i_flag || !x_flag) goto print_usage ;

    if (argc > 5) {
        if (strcmp(argv[arg_cnt], "@@") == 0) {
            covarg.input_type = ARG_FILENAME ;
            arg_cnt++ ;
        }
        else goto print_usage ;
    } 
    else {
        covarg.input_type = STDIN ;
    }

    return 0 ;

print_usage:
    perror("usage: ./funcov -i [input_dir] -x [executable_binary] [...]\n\nrequired\n-i : input directory path\n-x : executable binary path\n\noptional\n@@ : input type - file as an argument") ;
    return -1 ;
}

int
read_input_dir ()
{
    int file_cnt = 0 ;

    DIR * dir_ptr = 0x0 ;
    struct dirent * entry = 0x0 ;

    if ((dir_ptr = opendir(covarg.input_dir_path)) == 0x0) {
        perror("read_input_dir: opendir") ;
        return -1 ;
    }

    covarg.input_files = (input_t *) malloc(sizeof(input_t) * INPUT_CNT_UNIT) ;

    while ((entry = readdir(dir_ptr)) != 0x0) {
        if (file_cnt != 0x0 && file_cnt % INPUT_CNT_UNIT == 0) {
            covarg.input_files = realloc(covarg.input_files, sizeof(input_t) * (file_cnt / INPUT_CNT_UNIT + 1) * INPUT_CNT_UNIT) ;
            if (covarg.input_files == 0x0) {
                perror("read_input_dir: realloc") ;
                return -1 ;
            }
        }
        if (entry->d_name[0] != '.') {
            sprintf(covarg.input_files[file_cnt].file_path, "%s/%s", covarg.input_dir_path, entry->d_name) ;
            covarg.input_files[file_cnt].fun_cov = 0 ;
            file_cnt++ ;
        }
    }
    closedir(dir_ptr) ;

    covarg.input_file_cnt = file_cnt ;
    
    return 0 ;
}

int
main (int argc, char * argv[])
{
    if (get_cmd_args(argc, argv) == -1) return 1 ;
    if (read_input_dir() == -1) return 1 ;
    print_covarg() ;

    free(covarg.input_files) ;
    return 0 ;
}