#ifndef GETCOV
#define GETCOV

#include <stdint.h>
#include "funcov.h"

#define MAP_ROW_UNIT 4096
#define MAP_COL_UNIT 64
#define MAP_SIZE MAP_ROW_UNIT * MAP_COL_UNIT // Q.

typedef struct pair_entry {
    unsigned int hit_count ;
    char cov_string[BUF_SIZE] ; // callee,caller,line
} p_entry_t ;

typedef struct cov_stat {
    int id ;
    int exit_code ;
    unsigned int fun_coverage ;
    p_entry_t map[MAP_ROW_UNIT][MAP_COL_UNIT] ;
} cov_stat_t ;

int get_cov_stats (p_entry_t ** trace, cov_stat_t * stat, config_t * conf, int turn, int exit_code) ;

#endif