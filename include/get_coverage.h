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

typedef struct trace_bits {
    unsigned int bitmap_size ;
    uint8_t * bitmap ;
} trace_bits_t ;

int get_cov_stat (cov_stat_t * stat, config_t * conf, int turn, int exit_code) ;
void trace_cov_stat (unsigned int * trace_cov, trace_bits_t * trace_bits, cov_stat_t * cur_stat) ;

#endif