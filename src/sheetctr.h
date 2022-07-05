#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "sheetcomp.h"
#include "timekey.h"
#include "measure.h"
#include "color.h"
#include "note.h"
#include "log.h"

using std::pair;
using std::make_pair;
using std::vector;

class sheetController {
  public:

    void reset() {
      for (auto& disp : displayMeasure) {
        disp.reset();
      }
    }

    int getGlyphWidth(int codepoint, int size = fSize);

    void drawTimeSignature(const timeSig& time, int x, colorRGB col);
    
    void drawKeySignature(const keySig& key, int x, colorRGB col);
   
    int getKeyWidth(const keySig& key);
    int getTimeWidth(const timeSig& key);

    // no beaming
    void drawNote(const note& noteData, int x, colorRGB col);


    void disectMeasure(const measureController& measure);

    friend class midi;

  private:

    vector<sheetMeasure> displayMeasure;

    void findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType);
    int findTimePartWidth(const int part); 

    static constexpr int sharpHash[7]    = {1, 4, 0, 3, 6, 2, 5};
    static constexpr int sharpSpacing[7] = {0, 1, 0, 2, 4, 4, 5};
    static constexpr int sharpY[7]       = {-1, -1, -2, -2, 0, -1, -1};

    static constexpr int flatHash[7]     = {5, 2, 6, 3, 7, 4, 8};
    static constexpr int flatSpacing[7]  = {0, 0, 0, 1, 1, 2, 2};
    static constexpr int flatY[7]        = {0, -1, 0, -2, 0, -1, 0};

    // padding for thin "1" doesnt' work on second "1" (i.e. 11, 21, etc)
    // or singular "1"
    static constexpr int timeWidths[10] = {16, 14, 15, 15, 18, 15, 16, 15, 16, 16};
    
    static constexpr int sharpWidths[8] = {0 , 10, 21, 30, 42, 54, 64, 75};
    static constexpr int flatWidths[8]  = {0 ,  8, 18, 28, 39, 49, 60, 70};
    static constexpr int fSize = 157;
    static constexpr int accConstSpacing = 10;
    static constexpr int accMax = 7;

    static constexpr int borderSpacing = 15;
};
