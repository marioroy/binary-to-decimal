
/*
 * Output the factorial of n via mpz_out_str to standard output.
 *
 * Add -I/usr/local/include -L/usr/local/lib or other path if needed.
 * gcc -DUSE_GMP  -O2 -fopenmp fac_test.c -o fac_test -lgmp  -lm
 * gcc -DUSE_GMP  -O2 -pthread fac_test.c -o fac_test -lgmp  -lm
 *
 * gcc -DUSE_MPIR -O2 -fopenmp fac_test.c -o fac_test -lmpir -lm
 * gcc -DUSE_MPIR -O2 -pthread fac_test.c -o fac_test -lmpir -lm
 *
 * time ./fac_test 7200000 > out
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#if !defined(WIN32)
# include <sys/time.h>
#endif

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

// clock_gettime isn't available on some platforms, e.g. Darwin
//
// https://blog.habets.se/2010/09/
//   gettimeofday-should-never-be-used-to-measure-time.html

#if defined(__GNUC__) && !defined(__GNUC_VERSION__)
# define __GNUC_VERSION__ (__GNUC__ * 10000 + __GNUC_MINOR__ * 100)
#endif

double wall_clock ()
{
#if !defined(CLOCK_MONOTONIC) || (defined(__GNUC__) && __GNUC_VERSION__ < 40800)
  struct timeval timeval;

  (void) gettimeofday (&timeval, (void *) 0);
  return (double) timeval.tv_sec +
         (double) timeval.tv_usec / 1000000.0;
#else
  struct timespec timeval;

  (void) clock_gettime (CLOCK_MONOTONIC, &timeval);
  return (double) timeval.tv_sec +
         (double) timeval.tv_nsec / 1000000000.0;
#endif
}

void fact (int n)
{
  double wbegin, wend;
  mpz_t  p;

  wbegin = wall_clock();
  mpz_fac_ui(p, n);
  wend = wall_clock();

  fprintf(stderr, "mpz_fac_ui  : %9.3f secs.\n", wend - wbegin);
  fflush(stderr);

  wbegin = wall_clock();
  mpz_out_str(stdout, 10, p);
  wend = wall_clock();

  fprintf(stderr, "mpz_out_str : %9.3f secs.\n", wend - wbegin);
  fflush(stderr);

  mpz_clear(p);
}

int main (int argc, char *argv[])
{
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

