#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "timekey.h"
#include "color.h"
#include "log.h"

using std::pair;
using std::make_pair;
using std::vector;

class sheetController {
  public:
    sheetController() { 
      timeSignatureMap = {};
      keySignatureMap = {};
    }

    void reset();

    void addTimeSignature(int position, int tick, timeSig timeSignature);
    void addKeySignature(int position, int tick, keySig keySignature);

    void linkKeySignatures();

    timeSig getTimeSignature(int offset);
    keySig getKeySignature(int offset);
    
    keySig eventToKeySignature(int keySigType, bool isMinor);

    void drawTimeSignature(pair<int, int> sig, int x, colorRGB col);

    friend class midi;

  private:
    vector<pair<int, timeSig>> timeSignatureMap;
    vector<pair<int, keySig>> keySignatureMap;
};
