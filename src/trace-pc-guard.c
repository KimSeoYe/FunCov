#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <sanitizer/coverage_interface.h>

#include "../include/shm_coverage.h"  // TODO. edit README

#define BT_BUF_SIZE 5
#define STR_BUFF 512

static cov_stat_t * curr_stat ; // shm
static int curr_stat_shmid ;

/**
 * README
 * 
 * You need to use "this file" for sanitizer coverage.
 * ...
*/

extern void 
__sanitizer_cov_trace_pc_guard_init(uint32_t *start, uint32_t *stop) 
{
  static uint64_t N;  
  if (start == stop || *start) return;  

  for (uint32_t *x = start; x < stop; x++)
    *x = ++N;  

  curr_stat_shmid = get_shm(CURR_KEY, sizeof(cov_stat_t)) ;
}

/**
 * strings format
 * /home/kimseoye/git/FunCov/test/simple_example/example(negative+0x17) [0x512437]
 * /home/kimseoye/git/FunCov/test/simple_example/example(main+0x1a5) [0x512685]
*/

void
parse_string (char * cov_string, char ** strings)
{
  char callee[PATH_MAX] ;
  char caller[PATH_MAX] ;
  strcpy(callee, strings[2]) ;
  strcpy(caller, strings[3]) ;

  char callee_name[STR_BUFF] ;
  char caller_name[STR_BUFF] ;
  char caller_pc[STR_BUFF] ;

  char * tok ;
  char * next ;

  tok = strtok_r(callee, "(", &next) ;
  tok = strtok_r(NULL, "+", &next) ;
  strcpy(callee_name, tok) ;

  tok = strtok_r(caller, "(", &next) ;
  tok = strtok_r(NULL, "+", &next) ;
  strcpy(caller_name, tok) ;

  tok = strtok_r(NULL, "[", &next) ;
  tok = strtok_r(NULL, "]", &next) ;
  strcpy(caller_pc, tok) ;

  sprintf(cov_string, "%s,%s,%s", callee_name, caller_name, caller_pc) ;
}

unsigned int
hash_function (char * cov_string)
{
  unsigned int value = 0 ;

  for (int i = 0; i < strlen(cov_string); i++) {
    value = cov_string[i] + 31 * value ;
  }

  return value % MAP_ROW_UNIT ;
}

void
get_coverage (char ** strings)
{
  curr_stat = attatch_shm(curr_stat_shmid) ;

  char cov_string[BUF_SIZE] ;
  parse_string(cov_string, strings) ;

  unsigned int id = hash_function(cov_string) ;
  if (curr_stat->shm_map.map[id][0].hit_count == 0) {
    curr_stat->shm_map.map[id][0].hit_count++ ;
    strcpy(curr_stat->shm_map.map[id][0].cov_string, cov_string) ;
  }
  else {
    int col_idx = ++(curr_stat->shm_map.collision_cnt[id]) ;
    curr_stat->shm_map.map[id][col_idx].hit_count++ ;
    strcpy(curr_stat->shm_map.map[id][col_idx].cov_string, cov_string) ;
  }

  detatch_shm(curr_stat) ;
}

extern void 
__sanitizer_cov_trace_pc_guard(uint32_t *guard) 
{
  if (!*guard) return;  

  size_t nptrs ;
  void * buffer[BT_BUF_SIZE] ;
  char ** strings ;

  nptrs = backtrace(buffer, BT_BUF_SIZE) ;
  strings = backtrace_symbols(buffer, nptrs) ;
  if (strings == 0x0) {
    perror("__sanitizer_cov_trace_pc_guard: backtrace_symbols") ;
    exit(1) ;
  }

  get_coverage(strings) ;
  
  free(strings) ;
}
