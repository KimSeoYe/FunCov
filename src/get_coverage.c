#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../include/get_coverage.h"

uint8_t * 
get_bitmap (cov_stat_t * stat, config_t * conf, int turn)
{
    char log_path[PATH_MAX + 8] ;
    sprintf(log_path, "%s/%s", conf->output_dir_path, LOGNAME) ;
    if (access(log_path, F_OK) == -1) {
        perror("get_bitmap: access: Cannot find cov.log") ;
        exit(1) ;
    }
    
    FILE * fp = fopen(log_path, "rb") ;
    if (fp == 0x0) {
        perror("get_bitmap: fopen") ;
        exit(1) ;
    }

    char buf[BUF_SIZE] ;
    for (int line = 0; fgets(buf, BUF_SIZE, fp) > 0; line++) { 
        if (line != 0) {
            int guard = atoi(buf) ; 
            if (guard > stat->bitmap_size) {
                stat->bitmap_size = (guard / MAP_SIZE_UNIT) * MAP_SIZE_UNIT ;
                stat->bitmap = realloc(stat->bitmap, sizeof(uint8_t) * stat->bitmap_size) ;
                printf("realloc: %d\n", stat->bitmap_size) ;
            }

            int index = guard - 1 ; // because guard var. starts from 1.
            if (stat->bitmap[index] == 0) stat->fun_coverage++ ;
            stat->bitmap[index] = 1 ;   // TODO. hit count?
        }
    }
    fclose(fp) ;

    return stat->bitmap ;
}

void
get_cov_stat (cov_stat_t * stat, config_t * conf, int turn, int exit_code)
{
    stat->id = turn ;
    stat->exit_code = exit_code ;

    stat->bitmap = get_bitmap(stat, conf, turn) ;
}

 // trace_cov_stat(trace_cov, trace_bits, &cov_stats[turn]) ;
void
trace_cov_stat (unsigned int * trace_cov, trace_bits_t * trace_bits, cov_stat_t * cur_stat)
{
    if (cur_stat->bitmap_size > trace_bits->bitmap_size) {
        trace_bits->bitmap = realloc(trace_bits->bitmap, sizeof(uint8_t) * cur_stat->bitmap_size) ;
        trace_bits->bitmap_size = cur_stat->bitmap_size ;
    }

    *trace_cov = 0 ;
    for (int i = 0; i < trace_bits->bitmap_size; i++) {
        if (trace_bits->bitmap[i] == 0 && cur_stat->bitmap[i] != 0) {
            trace_bits->bitmap[i] = cur_stat->bitmap[i] ;
        }
        if (trace_bits->bitmap[i] != 0) (*trace_cov)++ ;
    }
}