#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include <vector>
#include "controller.h"

using std::vector;


extern controller ctr;
extern vector<int> codepointSet;
//extern enum class codepoints;
