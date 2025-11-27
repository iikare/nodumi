#pragma once

#include "enum.h"
#include "note.h"

struct sheetNote {
  int displayBegin;  // start of note quantization
  int displayAcc;
  int stave;
  bool left;
  bool visible;
  int oriNote;
};
