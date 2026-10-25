#pragma once

#include "enum.h"
#include "note.h"

struct sheetNote {
  int displayBegin;  // start of note quantization, in 64ths from the measure start
  int displayAcc;
  int stave;
  bool left;
  bool visible;
  bool out_of_bounds;
  int oriNote;
  int display_type;  // value of this piece, which differs from the source note once split
  bool tie_in;       // continues the previous piece of the same note
  bool tie_out;      // continues into the next piece of the same note
  bool tie_up;
  int acc_col;  // accidental column, counted leftwards from the notehead
  bool tie_outside;  // tied around the noteheads rather than between them
  int voice;  // 0 for the upper (or only) voice of the stave, 1 for the lower
};
