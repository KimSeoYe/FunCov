#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sanitizer/coverage_interface.h>

/**
 * README
 * 
 * You need to use "this file" for sanitizer coverage.
 * ...
*/

// This callback is inserted by the compiler as a module constructor
// into every DSO. 'start' and 'stop' correspond to the
// beginning and end of the section with the guards for the entire
// binary (executable or DSO). The callback will be called at least
// once per DSO and may be called multiple times with the same parameters.
extern void __sanitizer_cov_trace_pc_guard_init(uint32_t *start,
                                                    uint32_t *stop) {
  static uint64_t N;  // Counter for the guards.
  if (start == stop || *start) return;  // Initialize only once.
  
  FILE * fp = fopen("cov.log", "wb") ;
  char buf[1024] ;
  sprintf(buf, "INIT: %p %p\n", start, stop);
  fwrite(buf, strlen(buf), 1, fp) ;
  fclose(fp) ;
  
  for (uint32_t *x = start; x < stop; x++)
    *x = ++N;  // Guards should start from 1.
}

// This callback is inserted by the compiler on every edge in the
// control flow (some optimizations apply).
// Typically, the compiler will emit the code like this:
//    if(*guard)
//      __sanitizer_cov_trace_pc_guard(guard);
// But for large functions it will emit a simple call:
//    __sanitizer_cov_trace_pc_guard(guard);
extern void __sanitizer_cov_trace_pc_guard(uint32_t *guard) {
  if (!*guard) return;  // Duplicate the guard check.
  // If you set *guard to 0 this code will not be called again for this edge.
  // Now you can get the PC and do whatever you want:
  //   store it somewhere or symbolize it and print right away.
  // The values of `*guard` are as you set them in
  // __sanitizer_cov_trace_pc_guard_init and so you can make them consecutive
  // and use them to dereference an array or a bit vector.
  void *PC = __builtin_return_address(0);
  char PcDescr[1024];
  // This function is a part of the sanitizer run-time.
  // To use it, link with AddressSanitizer or other sanitizer.
  __sanitizer_symbolize_pc(PC, "PC:%p fun_name:%F loc_info:%L", PcDescr, sizeof(PcDescr));

  FILE * fp = fopen("cov.log", "ab") ;
  char log[2048] ;
  sprintf(log, "%d[%p] %s\n", *guard, guard, PcDescr) ;
  fwrite(log, strlen(log), 1, fp) ;
  fclose(fp) ;
}
