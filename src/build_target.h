#if defined(TARGET_REL)
  #define NO_DEBUG
#endif

#if defined(TARGET_WIN)
  #include "../dpd/raylib/src/raylib.h"
  #include "../dpd/raylib/src/rlgl.h"
  #define _USE_MATH_DEFINES
#else
  #include <raylib.h>
  #include <rlgl.h>
#endif
