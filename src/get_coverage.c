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
            stat->bitmap[index] = 1 ;
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