#ifndef GETCOV
#define GETCOV

#include "shm_coverage.h"

unsigned int count_coverage (map_elem_t * shm_map) ;
unsigned int get_trace_coverage(map_elem_t * trace_map, cov_stat_t * curr_stat) ;

#endif