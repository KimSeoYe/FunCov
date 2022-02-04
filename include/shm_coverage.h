#ifndef SHMCOV
#define SHMCOV

#include <stdint.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "funcov.h"

#define CURR_KEY 1111
#define TRACE_KEY 2222

#define MAP_ROW_UNIT 4096
#define MAP_COL_UNIT 64
#define MAP_SIZE MAP_ROW_UNIT * MAP_COL_UNIT // Q.

typedef struct map_elem {
    unsigned int hit_count ;
    char cov_string[BUF_SIZE] ; // "callee,caller,line"
} map_elem_t ;

typedef struct shm_map {
    map_elem_t map[MAP_ROW_UNIT][MAP_COL_UNIT] ;
    unsigned int collision_cnt[MAP_ROW_UNIT] ;
} shm_map_t ;

typedef struct cov_stat {
    int id ;
    int exit_code ;
    unsigned int fun_coverage ;
    shm_map_t shm_map ;
} cov_stat_t ;

int get_shm (int key, int type_size) ;
void * attatch_shm (int shm_id) ;
void detatch_shm (void * shm_addr) ;
void remove_shm (int shm_id) ;

#endif