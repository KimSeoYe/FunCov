#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <sanitizer/coverage_interface.h>

#define BT_BUF_SIZE 5

/**
 * README
 * 
 * You need to use "this file" for sanitizer coverage.
 * ...
*/

extern void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,
                                                    uint32_t *stop) {
  static uint64_t N;  // Counter for the guards.
  if (start == stop || *start) return;  // Initialize only once.
  
  FILE * fp = fopen("cov.log", "wb") ;
  char buf[1024] ;
  sprintf(buf, "CALLEE:file_name(func_name+offset) [PC]::CALLER:file_name(func_name+offset) [PC]\n");
  fwrite(buf, strlen(buf), 1, fp) ;
  fclose(fp) ;
  
  for (uint32_t *x = start; x < stop; x++)
    *x = ++N;  // Guards should start from 1.
}


extern void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
  if (!*guard) return;  // Duplicate the guard check.

  size_t nptrs ;
  void * buffer[BT_BUF_SIZE] ;
  char ** strings ;

  nptrs = backtrace(buffer, BT_BUF_SIZE) ;
  strings = backtrace_symbols(buffer, nptrs) ;
  if (strings == 0x0) {
    perror("__sanitizer_cov_trace_pc_guard: backtrace_symbols") ;
    exit(1) ;
  }

  FILE * fp = fopen("cov.log", "ab") ;
  char log[2048] = "" ;
  sprintf(log, "CALLEE:%s::CALLER:%s\n", strings[2], strings[3]) ;
  fwrite(log, strlen(log), 1, fp) ;
  free(strings) ;

  fclose(fp) ;
}
