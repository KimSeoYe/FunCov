#include "../include/get_coverage.h"
#include <string.h>
#include <stdio.h>

unsigned int
count_coverage (map_elem_t * shm_map)
{
    int cov = 0 ;

    for (int i = 0; i < MAP_SIZE; i++) {
        if (shm_map[i].hit_count != 0) cov++ ;
    }

    return cov ;
}

unsigned int
get_trace_coverage(map_elem_t * trace_map, cov_stat_t * curr_stat)
{
    for (int i = 0; i < MAP_SIZE; i++) {
        if (curr_stat->map[i].hit_count == 0) continue ;

        if (trace_map[i].hit_count == 0) {
            trace_map[i].hit_count += curr_stat->map[i].hit_count ;
            strcpy(trace_map[i].cov_string, curr_stat->map[i].cov_string) ;
        }
        else if (strcmp(trace_map[i].cov_string, curr_stat->map[i].cov_string) == 0) {
            trace_map[i].hit_count += curr_stat->map[i].hit_count ;
        }
    }

    return count_coverage(trace_map) ;
}