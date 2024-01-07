#if defined(TARGET_REL)
  #define NO_DEBUG
  #define PRAGMA(X) _Pragma(#X)
  #define OPENMP_USE_SIMD PRAGMA(omp simd)
#else 
  #define PRAGMA(X) ;
  #define OPENMP_USE_SIMD ; 
#endif

#if defined(TARGET_WIN)
  #include "../dpd/raylib/src/raylib.h"
  #include "../dpd/raylib/src/rlgl.h"
  #define _USE_MATH_DEFINES
#else
  #include <raylib.h>
  #include <rlgl.h>
#endif
