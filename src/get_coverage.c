#include <stdio.h>

#include "../include/get_coverage.h"

uint8_t * 
get_bitmap (cov_stat_t * stat, config_t * conf, int turn)
{

    return stat->bitmap ;
}

void
get_cov_stat (cov_stat_t * stat, config_t * conf, int turn, int exit_code)
{
    stat->id = turn ;
    stat->exit_code = exit_code ;

    // get bitmap by parsing the log
    stat->bitmap = get_bitmap(stat, conf, turn) ;

    // set fun_coverage & bitmap_size
}