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
    sheetController() { 
    }

    void drawTimeSignature(pair<int, int> sig, int x, colorRGB col);
    
    void drawKeySignature(keySig key, int x, colorRGB col);
    
    // no beaming
    void drawNote(sheetNote noteData, int x, colorRGB col);

    friend class midi;

  private:
};
