#ifndef GETCOV
#define GETCOV

#include "shm_coverage.h"

unsigned int count_coverage (map_elem_t shm_map[][MAP_COL_UNIT]) ;
unsigned int get_trace_coverage(map_elem_t trace_map[][MAP_COL_UNIT], cov_stat_t * curr_stat) ;

#endif