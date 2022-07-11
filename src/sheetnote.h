#pragma once

#include "note.h"
#include "enum.h"

struct sheetNote {
  int displayBegin; // start of note quantization
  int displayAcc;
  note* oriNote;
};

