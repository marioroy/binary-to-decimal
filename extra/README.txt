//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Motivation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Parallel recursion for mpn_get_str supporting OpenMP and pthreads.
By Mario Roy, 2018. See https://github.com/marioroy/binary-to-decimal.

Dear GMP/MPIR developers,

I tried the parallel pgmp-chudnovsky.c on the web and wanted mpn_get_str
to do the conversion faster. For really "big" numbers, it still takes a
long time before the first divide-and-conquer inside mpn/get_str.c.
At which point 2 threads run, then 4, and eventually 8 threads max.

What will become of GMP/MPIR when computing devices are later equipped
with 100 cores and the clock speed decreases due to having many cores?
Does that mean applications will run slower over time due to using
one core?

  a wish : gmp-7.x.x   parallel
           mpir-4.x.x  parallel

A common wish on the web is for mpn_get_str to run faster. Please,
feel free to disregard my first and humble attempt. Another way is
deeper inside the library. E.g. having mpn_mul run parallel.

Acknowledgemet:
  https://github.com/anthay/binary-to-decimal, by Anthony Hay.

  Useful is prime_test.cpp for validating changes to mpn/get_str.c.
  Added prime5.cpp (using MPIR) and prime6.cpp (using GMP).


Regards,
  Mario Roy

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Enable parallel by including relevant header files inside C/C++ code.
// Likewise, add CFLAGS option.
//
//   gcc/g++ -fopenmp ...
//   gcc/g++ -pthread ...
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// GMP ( tested with gmp-5.1.0a minimum - gmp-6.1.2 )
//     ( also, benefits mpfr using gmp >= 5.1.0a    )

#include <gmp.h>

#if defined(_OPENMP)
# include "extra/gmp/mpn_get_str_omp.c"
# include "extra/gmp/mpf_get_str.c"
# include "extra/gmp/mpz_get_str.c"
#else
# include "extra/gmp/mpn_get_str_thr.c"
# include "extra/gmp/mpf_get_str.c"
# include "extra/gmp/mpz_get_str.c"
#endif

// MPIR ( tested with mpir-2.6.0 minimum - mpir-3.0.0 )

#include <mpir.h>

#if defined(_OPENMP)
# include "extra/mpir/mpn_get_str_omp.c"
# include "extra/mpir/mpf_get_str.c"
# include "extra/mpir/mpz_get_str.c"
#else
# include "extra/mpir/mpn_get_str_thr.c"
# include "extra/mpir/mpf_get_str.c"
# include "extra/mpir/mpz_get_str.c"
#endif

// One may choose mpn_get_str_thr.c for OpenMP. That works too.

#if defined(_OPENMP)
# include "extra/gmp/mpn_get_str_thr.c"
# ...
#endif

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ls -R extra/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

README.txt gmp mpir

extra/gmp:
COPYING        longlong.h        mpn_get_str_omp.c mpz_get_str.c
gmp-impl.h     mpf_get_str.c     mpn_get_str_thr.c

extra/mpir:
COPYING        longlong.h        mpn_get_str_thr.c x86_64
arm            mpf_get_str.c     mpz_get_str.c
gmp-impl.h     mpn_get_str_omp.c x86

extra/mpir/arm:
longlong.h

extra/mpir/x86:
longlong.h

extra/mpir/x86_64:
longlong.h

The unchanged files longlong.h, mpf_get_str.c, and mpz_get_str.c are 
taken from the baseline tree. They are placed here so that the build
and runtime picks up mpf/mpz get_str and subsequently mpn get_str
from here versus the same function from libgmp/libmpir.

The gmp-impl.h file is shrunked down lots. I left only the relevant
bits necessary for building successfully.

If desired, updating the baseline tree requires just one file change.

  cp extra/gmp/mpn_get_str_thr.c gmp-6.1.2/mpn/get_str.c
  cp extra/mpir/mpn_get_str_thr.c mpir-3.0.0/mpn/get_str.c

It requires adding -pthread to CFLAGS somewhere. The pthread solution
runs on platforms lacking OpenMP support. It also works with other
languages not built with OpenMP. E.g. Perl.

// Cheers.

