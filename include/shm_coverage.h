#ifndef SHMCOV
#define SHMCOV

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "funcov.h"

#define CURR_KEY 1010

// #define MAP_ROW_UNIT 65536   // WARNING: memory limit...
// #define MAP_COL_UNIT 2
// #define MAP_SIZE MAP_ROW_UNIT * MAP_COL_UNIT 
#define MAP_SIZE 65536
#define COV_STRING_MAX 512

typedef struct map_elem {
    uint8_t hit_count ;
    char cov_string[COV_STRING_MAX] ; // "callee,caller,PC"
} map_elem_t ;

typedef struct cov_stat {
    int id ;
    int exit_code ;
    unsigned int fun_coverage ;
    // map_elem_t map[MAP_ROW_UNIT][MAP_COL_UNIT] ;
    map_elem_t map[MAP_SIZE] ;
} cov_stat_t ;

int get_shm (int key, int type_size) ;
void * attatch_shm (int shm_id) ;
void detatch_shm (void * shm_addr) ;
void remove_shm (int shm_id) ;

#endif