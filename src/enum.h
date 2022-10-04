#pragma once

#include <vector>
#include <algorithm>
#include "aghcpenum.h"

using std::find;
using std::vector;
using std::is_enum;

template <class T>
class enumChecker {
  public:

    enumChecker() : items(0), itemsLastFrame(0) {
      static_assert(is_enum<T>::value, "class requires an enum type");
    }

    void add(T item) {
      if (!contains(item)) {
        items.push_back(item);
      }
    }

    void remove(T item) {
      items.remove(item);
    }

    void clear() {
      itemsLastFrame = items;
      items.clear();
    }

    bool contains(T item) {
      return find(items.begin(), items.end(), item) != items.end(); 
    }

    bool contains(T item1, T item2...) {
      return contains(item1) || contains(item2);
    }
    
    bool containsLastFrame(T item) {
      return find(itemsLastFrame.begin(), itemsLastFrame.end(), item) != itemsLastFrame.end(); 
    }

    bool containsLastFrame(T item1, T item2...) {
      return containsLastFrame(item1) || containsLastFrame(item2);
    }


  private:
    vector<T> items;
    vector<T> itemsLastFrame;
};

enum assetType {
  ASSET_FONT,
  ASSET_RENDERTEXTURE,
  ASSET_TEXTURE,
  ASSET_SHADER,
  ASSET_IMAGE,
};

enum lerpType {
  INT_LINEAR,
  INT_ILINEAR,
  INT_QUADRATIC,
  INT_IQUADRATIC,
  INT_SSTEP,
  INT_CIRCULAR,
  INT_ICIRCULAR,
  INT_SINE,
  INT_ISINE
};

enum fileType {
  FILE_MIDI,
  FILE_MKI,
  FILE_NONE
};

enum schemeType {
  SCHEME_TRACK,
  SCHEME_TONIC,
  SCHEME_KEY,
  SCHEME_NONE
};

enum selectType {
  SELECT_BG,
  SELECT_LINE,
  SELECT_NOTE,
  SELECT_SHEET,
  SELECT_MEASURE,
  SELECT_NONE
};

enum displayType {
  DISPLAY_LINE,
  DISPLAY_BAR,
  DISPLAY_BALL,
  DISPLAY_PULSE
};

enum menuType {
  TYPE_MAIN,
  TYPE_SUB,
  TYPE_RIGHT,
  TYPE_RIGHTSUB,
  TYPE_COLOR
};

enum songTimeType {
  SONGTIME_RELATIVE,
  SONGTIME_ABSOLUTE,
  SONGTIME_NONE
};

enum hoverType {
  HOVER_NOW,
  HOVER_NOTE,
  HOVER_IMAGE,
  HOVER_MEASURE,
  HOVER_LINE,
  HOVER_SHEET,
  HOVER_MENU,
  HOVER_DIALOG,
  HOVER_BG, //default, should not be added
  HOVER_NONE
};

enum colorType {
  COLOR_PART,
  COLOR_VELOCITY,
  COLOR_TONIC
};

enum imageType {
  IMAGE_PNG,
  IMAGE_JPG,
  IMAGE_NONE
};

enum sheetEventType {
  EVENT_TIME,
  EVENT_KEY,
  EVENT_NOTE,
  EVENT_REST,
  EVENT_NONE
};

// in ratios of TPQ
enum noteType {
  NOTE_LARGE,            // 0  |  8 (fake)
  NOTE_WHOLE_DOT,        // 1  |  6
  NOTE_WHOLE,            // 2  |  4
  NOTE_HALF_DOT,         // 3  |  3
  NOTE_HALF,             // 4  |  2
  NOTE_QUARTER_DOT,      // 5  |  1.5
  NOTE_QUARTER,          // 6  |  1 
  NOTE_8_DOT,            // 7  |  0.75
  NOTE_8,                // 8  |  0.5
  NOTE_16_DOT,           // 9  |  0.375
  NOTE_16,               // 10 |  0.25
  NOTE_32,               // 11 |  0.125
  NOTE_64,               // 12 |  0.0625
  NOTE_NONE              // no numerical mapping
};

enum accidentalType{
  ACC_NATURAL,
  ACC_SHARP,
  ACC_FLAT,
  ACC_NONE
};

enum keySignatureType {
  KEYSIG_C,
  KEYSIG_CSHARP,
  KEYSIG_DFLAT,
  KEYSIG_D,
  KEYSIG_EFLAT,
  KEYSIG_E,
  KEYSIG_F,
  KEYSIG_FSHARP,
  KEYSIG_GFLAT,
  KEYSIG_G,
  KEYSIG_AFLAT,
  KEYSIG_A,
  KEYSIG_BFLAT,
  KEYSIG_B,
  KEYSIG_CFLAT,
  KEYSIG_NONE
};

enum staveType {
  STAVE_TREBLE,
  STAVE_BASS,
  STAVE_NONE
};

enum displayDirectionType {
  DISP_UP,
  DISP_DOWN,
  DISP_NONE
};

enum displayAccStateType {
  DA_STATE_CLEAR,
  DA_STATE_SHARP,
  DA_STATE_FLAT,
  DA_STATE_NATURAL,
  DA_STATE_SHARP_MULT,
  DA_STATE_FLAT_MULT,
  DA_STATE_NONE
};

enum flagType {
  FLAGTYPE_STEM,
  FLAGTYPE_8,
  FLAGTYPE_16,
  FLAGTYPE_32,
  FLAGTYPE_64,
  FLAGTYPE_NONE
};

enum flagDirType {
  FLAG_UP,
  FLAG_DOWN,
  FLAG_NONE
};
