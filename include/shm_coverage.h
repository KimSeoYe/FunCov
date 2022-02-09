#ifndef SHMCOV
#define SHMCOV

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "funcov.h"

#define CURR_KEY 1010

#define MAP_ROW_UNIT 2048   // WARNING: memory limit...
#define MAP_COL_UNIT 64
#define MAP_SIZE MAP_ROW_UNIT * MAP_COL_UNIT 

typedef struct map_elem {
    unsigned int hit_count ;
    char cov_string[BUF_SIZE] ; // "callee,caller,PC"
} map_elem_t ;

typedef struct cov_stat {
    int id ;
    int exit_code ;
    unsigned int fun_coverage ;
    map_elem_t map[MAP_ROW_UNIT][MAP_COL_UNIT] ;
} cov_stat_t ;

int get_shm (int key, int type_size) ;
void * attatch_shm (int shm_id) ;
void detatch_shm (void * shm_addr) ;
void remove_shm (int shm_id) ;

#endif