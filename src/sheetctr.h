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
      sheetPageSeparator.clear();
      displayMeasure.clear();

    }

    int getGlyphWidth(int codepoint, int size = fSize);

    // TODO: merge into chord drawing function
    // no beaming
    void drawNote(const sheetNote& noteData, int x, colorRGB col);

    void disectMeasure(measureController& measure);
    void findSheetPages();


    void drawSheetPage();

    friend class midi;
    friend class sheetMeasure;

  private:

    vector<int> sheetPageSeparator;
    vector<sheetMeasure> displayMeasure;
    
    void drawTimeSignature(const timeSig& time, int x);
    void drawKeySignature(const keySig& key, int x);
   
    int getKeyWidth(const keySig& key);
    int getTimeWidth(const timeSig& key);
    int getSymbolWidth(const int symbol);
    int getSymbolWidth(const int flagType, const int dir);
    int getSymbolType(const int noteType);

    void findDFAStartVector(vector<int>& DFAState, const keySig& ks); 
    void findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType);
    int findTimePartWidth(const int part); 
    int findStaveY(int sheetY, int stave);
    int findMeasureWidth(int measureNum, bool includeSig = false);
    pair<int, int> findSheetPageLimit(int measureNum) const;

    static constexpr pair<int, int> getStaveRenderLimit() { return make_pair(staveFlagLimitMap[0][0], staveFlagLimitMap[1][0]); }
    static constexpr int getFlagLimit(int flagType, int stave) {
      return staveFlagLimitMap[stave][flagType == FLAGTYPE_NONE ? 0 : (flagType+1) % 6];
    }
    int getDisplayAccType(int& DFAState, int noteAccType);
    int mapSheetY(int sheetY);

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
    
    static constexpr int stemWidth = 2;
    static constexpr int dotSpacing = 4;
    static constexpr int accSpacing = 4;
    static constexpr int sigSpacing = 4;
    
    static constexpr int placeholderWidth = 24;


    // order: 0, 4, 8, 16, 32, 64
    // limit is the sheetY that causes clipping when facing up
    // first value is the overall limit (above/below will not be rendered)
    static constexpr int staveFlagLimitMap[2][6] = {
                                                     { 22, 18, 18, 18, 17, 15},
                                                     {-22,-18,-18,-18,-15,-15},
                                                   };

    // indexed to middle C (val:60, MIDC_NOTE_IDX)
    static constexpr int staveKeySigMap[15][7] = {
                                                   { // KEYSIG_C 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_CSHARP  
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_SHARP, 
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_SHARP, 
                                                   },
                                                   { // KEYSIG_DFLAT 
                                                     DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_CLEAR, 
                                                     DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_D  
                                                     DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_SHARP, 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_EFLAT
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_CLEAR, 
                                                     DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_E  
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_SHARP, 
                                                     DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_F
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_FSHARP  
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_SHARP, 
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_GFLAT
                                                     DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_CLEAR, 
                                                     DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_G
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_SHARP, 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_AFLAT  
                                                     DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_CLEAR, 
                                                     DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_A
                                                     DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_SHARP, 
                                                     DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_BFLAT
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_FLAT, DA_STATE_CLEAR, 
                                                     DA_STATE_CLEAR, DA_STATE_CLEAR, DA_STATE_FLAT, 
                                                   },
                                                   { // KEYSIG_B 
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_CLEAR, DA_STATE_SHARP, 
                                                     DA_STATE_SHARP, DA_STATE_SHARP, DA_STATE_CLEAR, 
                                                   },
                                                   { // KEYSIG_CFLAT 
                                                     DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                     DA_STATE_FLAT, DA_STATE_FLAT, DA_STATE_FLAT, 
                                                   },
                                                 };
    static constexpr int staveKeySigMapLen = 7;
};
