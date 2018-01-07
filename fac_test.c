
// gcc -DUSE_GMP  -O2 -fopenmp fac_test.c -o fac_test -lgmp  -lm
// gcc -DUSE_GMP  -O2 -pthread fac_test.c -o fac_test -lgmp  -lm
//
// gcc -DUSE_MPIR -O2 -fopenmp fac_test.c -o fac_test -lmpir -lm
// gcc -DUSE_MPIR -O2 -pthread fac_test.c -o fac_test -lmpir -lm
//
// time ./fac_test 7200000 > out

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if defined(USE_GMP) || !defined(USE_MPIR)
 #include <gmp.h>
 #if defined(_OPENMP)
 # include "extra/gmp/mpn_get_str_omp.c"
 # include "extra/gmp/mpz_out_str.c"
 #else
 # include "extra/gmp/mpn_get_str_thr.c"
 # include "extra/gmp/mpz_out_str.c"
 #endif
#else
 #include <mpir.h>
 #if defined(_OPENMP)
 # include "extra/mpir/mpn_get_str_omp.c"
 # include "extra/mpir/mpz_out_str.c"
 #else
 # include "extra/mpir/mpn_get_str_thr.c"
 # include "extra/mpir/mpz_out_str.c"
 #endif
#endif

void fact (int n) {
  mpz_t p; mpz_fac_ui(p, n);

 #if !defined(NO_OUTPUT)
  mpz_out_str(stdout, 10, p);
 #endif

  mpz_clear(p);
}

int main (int argc, char *argv[]) {
  int n;

  if (argc <= 1) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
  }
  n = atoi(argv[1]);
  assert(n >= 0);
  fact(n);

  return 0;
}

