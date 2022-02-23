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
get_trace_coverage (unsigned int prev_cov, map_elem_t * trace_map, cov_stat_t * curr_stat)
{
    unsigned int new_cov = 0 ;

    for (int cur_idx = 0; cur_idx < MAP_SIZE; cur_idx++) {
        if (curr_stat->map[cur_idx].hit_count == 0) continue ;

        int trace_idx = cur_idx ; 
        int found = 0 ;
        for (int i = 0; i < MAP_SIZE; i++) {
            if (trace_map[trace_idx].hit_count == 0) {
                trace_map[trace_idx].hit_count += curr_stat->map[cur_idx].hit_count ;
                strcpy(trace_map[trace_idx].cov_string, curr_stat->map[cur_idx].cov_string) ;
                found = 1 ;
                new_cov++ ;
                break ;
            }
            else if (strcmp(trace_map[trace_idx].cov_string, curr_stat->map[cur_idx].cov_string) == 0) {
                trace_map[trace_idx].hit_count += curr_stat->map[cur_idx].hit_count ;
                found = 1 ;
                break ;
            }
            else trace_idx++ ;
        }
        if (!found) {
            perror("get_trace_coverage: Not found") ;
            return -1 ;
        }
    }

    return prev_cov + new_cov ;
}