#ifndef GETCOV
#define GETCOV

#include "shm_coverage.h"

unsigned int count_coverage (shm_map_t * shm_map) ;
unsigned int get_trace_coverage(shm_map_t * trace_map, cov_stat_t * curr_stat, unsigned int prev_cov) ;

#endif