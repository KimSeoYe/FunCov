#ifndef GETCOV
#define GETCOV

#include <stdint.h>
#include "funcov.h"

#define MAP_SIZE_UNIT 4096 // Q.

typedef struct cov_stat {
    int id ;
    int exit_code ;
    unsigned int fun_coverage ;
    unsigned int bitmap_size ;
    uint8_t * bitmap ;
} cov_stat_t ;

void get_cov_stat (cov_stat_t * stat, config_t * conf, int turn, int exit_code) ;

#endif