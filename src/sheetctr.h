#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "timekey.h"
#include "color.h"
#include "note.h"
#include "log.h"

using std::pair;
using std::make_pair;
using std::vector;

class sheetController {
  public:

    void drawTimeSignature(pair<int, int> sig, int x, colorRGB col);
    
    void drawKeySignature(keySig key, int x, colorRGB col);
   
    int getKeyWidth(keySig key);

    // no beaming
    void drawNote(sheetNote noteData, int x, colorRGB col);

    friend class midi;

  private:

    static constexpr int* keyWidths[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static constexpr int fSize = 157;
};
