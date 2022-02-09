#include "../include/get_coverage.h"
#include <string.h>
#include <stdio.h>

unsigned int
count_coverage (map_elem_t shm_map[][MAP_COL_UNIT])
{
    int cov = 0 ;

    for (int i = 0; i < MAP_ROW_UNIT; i++) {
        for (int j = 0; j < MAP_COL_UNIT; j++) {
            if (shm_map[i][j].hit_count == 0) break ;
            cov++ ;
        }
    }

    return cov ;
}

unsigned int
get_trace_coverage(map_elem_t trace_map[][MAP_COL_UNIT], cov_stat_t * curr_stat)
{
    for (int row = 0; row < MAP_ROW_UNIT; row++) {
        for (int col = 0; col < MAP_COL_UNIT; col++) {
            if (curr_stat->map[row][col].hit_count == 0) break ;
            
            for (int cursor = 0; cursor < MAP_COL_UNIT; cursor++) {
                if (strcmp(trace_map[row][cursor].cov_string, curr_stat->map[row][col].cov_string) == 0) {
                    trace_map[row][cursor].hit_count += curr_stat->map[row][col].hit_count ;
                    break ;
                } 
                else if (trace_map[row][cursor].hit_count == 0) {
                    trace_map[row][cursor].hit_count += curr_stat->map[row][col].hit_count ;
                    strcpy(trace_map[row][cursor].cov_string, curr_stat->map[row][col].cov_string) ;
                    break ;
                }
            }
        }
    }

    return count_coverage(trace_map) ;
}