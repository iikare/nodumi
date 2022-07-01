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

    int getGlyphWidth (int codepoint, int size = fSize);

    void drawTimeSignature(pair<int, int> sig, int x, colorRGB col);
    
    void drawKeySignature(const keySig& key, int x, colorRGB col);
   
    int getKeyWidth(const keySig& key);

    // no beaming
    void drawNote(sheetNote noteData, int x, colorRGB col);

    friend class midi;

  private:


    void findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType);
  

    vector<int> sharpHash    = {1, 4, 0, 3, 6, 2, 5};
    vector<int> sharpSpacing = {0, 1, 0, 2, 4, 4, 5};
    vector<int> sharpY       = {-1, -1, -2, -2, 0, -1, -1};

    vector<int> flatHash     = {5, 2, 6, 3, 7, 4, 8};
    vector<int> flatSpacing  = {0, 0, 0, 1, 1, 2, 2};
    vector<int> flatY        = {0, -1, 0, -2, 0, -1, 0};

    static constexpr int sharpWidths[8] = {0, 10, 21, 30, 42, 54, 64, 75};
    static constexpr int flatWidths[8]  = {0,  8, 18, 28, 39, 49, 60, 70};
    static constexpr int fSize = 157;
    static constexpr int accConstSpacing = 10;
};
