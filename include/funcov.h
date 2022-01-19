#ifndef COVENGINE
#define COVENGINE

#define PATH_MAX 4096
#define BUF_SIZE 1024

#define LOGNAME "cov.log"

typedef enum input_type { STDIN = 0, ARG_FILENAME } input_type_t ;

typedef struct input {
    char file_path[PATH_MAX] ;
    int fun_cov ; // Q. need ?
} input_t ;

typedef struct config {    // Q. don't need to use a struct?
    input_type_t input_type ;
    char binary_path[PATH_MAX] ;
    char input_dir_path[PATH_MAX] ;
    char output_dir_path[PATH_MAX] ;
    
    int input_file_cnt ;
    input_t * input_files ;
} config_t ;

#endif