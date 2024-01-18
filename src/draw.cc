#include "draw.h"

// valid label format:
//   | label[0]    | key
//   | label[1]    | accidental (if exist, "#" or "b")
//   | label[2..n] | not considered
void drawNoteLabel(string label, int x, int y, int size, int sym_size, const colorRGB& col) {
  if (label.length() > 0 && any_of(label[1], 'b', '#')) {
    drawTextEx(label.substr(0,1), x, y, col);
    x += measureTextEx(label.substr(0,0)).x + 6;

    int ksSym = SYM_ACC_SHARP;
    int ksWidth = 5;
    int ksYOffset = 31;
    if (label[1] == 'b') {
      ksSym = SYM_ACC_FLAT;
      ksWidth = 4;
      ksYOffset = 29;
    }
    drawSymbol(ksSym, sym_size, x, y-ksYOffset, col);
    x += ksWidth;
    
    drawTextEx(label.substr(2,label.length()-1), x, y, col, 255, size);
  }
  else {
    drawTextEx(label, x, y, col, 255, size);
  }
}
