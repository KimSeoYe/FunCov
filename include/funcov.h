#ifndef COVENGINE
#define COVENGINE

#define PATH_MAX 4096

typedef enum input { STDIN = 0, ARG_FILENAME } input_t ;

typedef struct cov_arg {
    input_t input_type ;
    char binary_path[PATH_MAX] ;
    char input_dir_path[PATH_MAX] ;
    // result directory name
} cov_arg_t ;

// TODO. cov_stat

#endif