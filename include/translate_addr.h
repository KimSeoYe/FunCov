#ifndef TRANSLATE
#define TRANSLATE

#include "funcov.h"

#define ADDR_MAX 16

typedef struct location {
    char pc_val[ADDR_MAX] ;
    char location[PATH_MAX] ;
} location_t ;

void translate_pc_values (location_t * translated_locations) ;

#endif