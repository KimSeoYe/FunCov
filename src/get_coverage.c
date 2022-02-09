#include "../include/get_coverage.h"
#include <string.h>

unsigned int
count_coverage (shm_map_t * shm_map)
{
    int cov = 0 ;

    for (int i = 0; i < MAP_ROW_UNIT; i++) {
        for (int j = 0; j < MAP_COL_UNIT; j++) {
            if (shm_map->map[i][j].hit_count == 0) break ;
            cov++ ;
        }
    }

    return cov ;
}

unsigned int
get_trace_coverage(shm_map_t * trace_map, cov_stat_t * curr_stat, unsigned int prev_cov)
{
    unsigned int trace_cov = prev_cov ;

    for (int i = 0; i < MAP_ROW_UNIT; i++) {
        trace_map->collision_cnt[i] = curr_stat->shm_map.collision_cnt[i] ; 

        for (int j = 0; j < MAP_COL_UNIT; j++) {
            if (curr_stat->shm_map.map[i][j].hit_count == 0) break ;
            if (trace_map->map[i][j].hit_count == 0) {
                strcpy(trace_map->map[i][j].cov_string, curr_stat->shm_map.map[i][j].cov_string) ;
                trace_cov++ ;
            }
            trace_map->map[i][j].hit_count += curr_stat->shm_map.map[i][j].hit_count ;
        }
    }

    return trace_cov ;
}