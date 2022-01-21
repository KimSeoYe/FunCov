#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "../include/get_coverage.h"

void
reallocate_maps (trace_t * trace, cov_stat_t * stat, int guard)
{
    stat->bitmap_size = (guard / MAP_SIZE_UNIT) * MAP_SIZE_UNIT ;
    stat->bitmap = realloc(stat->bitmap, sizeof(uint8_t) * stat->bitmap_size) ;

    trace->bitmap = realloc(trace->bitmap, sizeof(uint8_t) * stat->bitmap_size) ;
    trace->fun_names = realloc(trace->fun_names, sizeof(char *) * stat->bitmap_size) ;
    for (int i = trace->bitmap_size; i < stat->bitmap_size; i++) {
        trace->fun_names[i] = (char *) malloc(sizeof(char) * FUN_NAME_MAX) ;
    }

    trace->bitmap_size = stat->bitmap_size ;
}

void
update_fun_names (trace_t * trace, char * buf, int index)
{
    char * fun_pos = strstr(buf, ":in") ;
    if (fun_pos == 0x0) {
        perror("get_bitmap: strstr: LLVM symbolizer failed to find a function name") ;
        exit(1) ;
    }
    strncpy(trace->fun_names[index], fun_pos + 3, strlen(fun_pos + 3) - 1) ;
}

uint8_t * 
get_bitmap (trace_t * trace, cov_stat_t * stat, config_t * conf, int turn)
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
                reallocate_maps(trace, stat, guard) ;
            }

            int index = guard - 1 ; // because guard var. starts from 1.
            if (stat->bitmap[index] == 0) stat->fun_coverage++ ;
            stat->bitmap[index] = 1 ;   // TODO. hit count?

            if (trace->bitmap[index] == 0) {
                update_fun_names(trace, buf, index) ;
            }
        }
    }
    fclose(fp) ;

    return stat->bitmap ;
}

int
get_cov_value (cov_stat_t * stat)
{
    int cov_val = 0 ;
    for (int i = 0; i < stat->bitmap_size; i++) {
        if (stat->bitmap[i] != 0) cov_val++ ;
    }
    return cov_val ; 
}

int 
trace_cov_stat (trace_t * trace, cov_stat_t * cur_stat)
{
    int trace_cov = 0 ;
    for (int i = 0; i < trace->bitmap_size; i++) {
        if (trace->bitmap[i] == 0 && cur_stat->bitmap[i] != 0) {
            trace->bitmap[i] = cur_stat->bitmap[i] ;
        }
        if (trace->bitmap[i] != 0) trace_cov++ ;
    }

    return trace_cov ;
}

int
get_cov_stats (trace_t * trace, cov_stat_t * stat, config_t * conf, int turn, int exit_code)
{
    stat->id = turn ;
    stat->exit_code = exit_code ;

    stat->bitmap = get_bitmap(trace, stat, conf, turn) ;
    stat->fun_coverage = get_cov_value(stat) ;

    int trace_cov = trace_cov_stat(trace, stat) ;
    
    return trace_cov ;
}