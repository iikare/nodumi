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
  int display_type;  // undotted value of this piece, which differs from the source note once split
  int dots;          // augmentation dots on this piece
  bool tie_in;       // continues the previous piece of the same note
  bool tie_out;      // continues into the next piece of the same note
  bool tie_up;
  int acc_col;  // accidental column, counted leftwards from the notehead
  bool tie_outside;  // tied around the noteheads rather than between them
  int voice;  // 0 for the upper (or only) voice of the stave, 1 for the lower
  int sheet_y;  // staff position as drawn, an octave nearer the stave under an 8va or 8vb line
  int arpeggio;  // the rolled chord this piece starts, or -1
  int grace_of;  // the note this grace note leads into, drawn small before it, or -1 for a note taking time
};
