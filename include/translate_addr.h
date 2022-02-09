#ifndef TRANSLATE
#define TRANSLATE

#include "funcov.h"
#include "../include/get_coverage.h"

#define ADDR_MAX 16

typedef struct location {
    char pc_val[ADDR_MAX] ;
    char location[PATH_MAX] ;
} location_t ;

void translate_pc_values (location_t * translated_locations, int locations_cnt, map_elem_t trace_map[][MAP_COL_UNIT], char * binary_path) ;

#endif