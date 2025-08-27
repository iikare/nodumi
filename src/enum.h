#pragma once

#include <algorithm>
#include <vector>

#include "aghcpenum.h"

using std::find;
using std::is_enum;
using std::vector;

template <class T>
class enumChecker {
 public:
  enumChecker() : items(0), itemsLastFrame(0) { static_assert(is_enum<T>::value, "class requires an enum type"); }

  void add(T item) {
    if (!contains(item)) {
      items.push_back(item);
    }
  }

  void remove(T item) { items.remove(item); }

  void clear() {
    itemsLastFrame = items;
    items.clear();
  }

  bool contains(T item) { return find(items.begin(), items.end(), item) != items.end(); }

  bool contains(T item1, T item2...) { return contains(item1) || contains(item2); }

  bool containsLastFrame(T item) {
    return find(itemsLastFrame.begin(), itemsLastFrame.end(), item) != itemsLastFrame.end();
  }

  bool containsLastFrame(T item1, T item2...) { return containsLastFrame(item1) || containsLastFrame(item2); }

 private:
  vector<T> items;
  vector<T> itemsLastFrame;
};

enum class ASSET { FONT, RENDER_TEXTURE, TEXTURE, SHADER, IMAGE, NONE };

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

enum fileType { FILE_MIDI, FILE_MKI, FILE_NONE };

enum pathType { PATH_DATA, PATH_IMAGE, PATH_MKI, PATH_NONE };

enum schemeType { SCHEME_TRACK, SCHEME_TONIC, SCHEME_KEY, SCHEME_NONE };

enum selectType { SELECT_BG, SELECT_LINE, SELECT_NOTE, SELECT_SHEET, SELECT_MEASURE, SELECT_NONE };

enum displayType {
  DISPLAY_BAR,
  DISPLAY_LINE,
  DISPLAY_PULSE,
  DISPLAY_BALL,
  DISPLAY_FFT,
  DISPLAY_VORONOI,
  DISPLAY_LOOP,
  DISPLAY_NONE
};

enum menuType { TYPE_MAIN, TYPE_SUB, TYPE_RIGHT, TYPE_RIGHTSUB, TYPE_COLOR };

enum menuContentType {
  CONTENT_FILE,
  CONTENT_EDIT,
  CONTENT_VIEW,
  CONTENT_DISPLAY,
  CONTENT_SONG,
  CONTENT_MIDI,
  CONTENT_INPUT,
  CONTENT_OUTPUT,
  CONTENT_COLOR,
  CONTENT_SCHEME,
  CONTENT_INFO,
  CONTENT_PALETTE,
  CONTENT_RIGHT,
  CONTENT_COLORSELECT,
  CONTENT_NONE,
};

enum songTimeType { SONGTIME_RELATIVE, SONGTIME_ABSOLUTE, SONGTIME_NONE };

enum hoverType {
  HOVER_NOW,
  HOVER_NOTE,
  HOVER_IMAGE,
  HOVER_MEASURE,
  HOVER_LINE,
  HOVER_SHEET,
  HOVER_MENU,
  HOVER_DIALOG,
  HOVER_BG,  // default, should not be added
  HOVER_NONE
};

enum colorType { COLOR_PART, COLOR_VELOCITY, COLOR_TONIC };

enum imageType { IMAGE_PNG, IMAGE_JPG, IMAGE_NONE };

enum sheetEventType { EVENT_TIME, EVENT_KEY, EVENT_NOTE, EVENT_REST, EVENT_NONE };

// in ratios of TPQ
enum noteType {
  NOTE_LARGE,        // 0  |  8 (fake)
  NOTE_WHOLE_DOT,    // 1  |  6
  NOTE_WHOLE,        // 2  |  4
  NOTE_HALF_DOT,     // 3  |  3
  NOTE_HALF,         // 4  |  2
  NOTE_QUARTER_DOT,  // 5  |  1.5
  NOTE_QUARTER,      // 6  |  1
  NOTE_8_DOT,        // 7  |  0.75
  NOTE_8,            // 8  |  0.5
  NOTE_16_DOT,       // 9  |  0.375
  NOTE_16,           // 10 |  0.25
  NOTE_32,           // 11 |  0.125
  NOTE_64,           // 12 |  0.0625
  NOTE_NONE          // no numerical mapping
};

enum accidentalType { ACC_NATURAL, ACC_SHARP, ACC_FLAT, ACC_NONE };

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

enum staveType { STAVE_TREBLE, STAVE_BASS, STAVE_NONE };

enum displayDirectionType { DISP_UP, DISP_DOWN, DISP_NONE };

enum displayAccStateType {
  DA_STATE_CLEAR,
  DA_STATE_SHARP,
  DA_STATE_FLAT,
  DA_STATE_NATURAL,
  DA_STATE_SHARP_MULT,
  DA_STATE_FLAT_MULT,
  DA_STATE_NONE
};

enum flagType { FLAGTYPE_STEM, FLAGTYPE_8, FLAGTYPE_16, FLAGTYPE_32, FLAGTYPE_64, FLAGTYPE_NONE };

enum flagDirType { FLAG_UP, FLAG_DOWN, FLAG_NONE };

enum class ACTION {
  OPEN,
  OPEN_IMAGE,
  CLOSE,
  CLOSE_IMAGE,
  SAVE,
  SAVE_AS,
  RELOAD,
  EXIT,
  SHEET,
  INFO,
  FILE_INFO,
  PREFERENCES,
  LIVEPLAY,
  CHANGE_MODE,
  NAV_HOME,         // gg
  NAV_END,          // G
  NAV_SET_MEASURE,  // #G
  NAV_NEXT,
  NAV_NEXT_FAST,
  NAV_NEXT_MEASURE,
  NAV_PREV,
  NAV_PREV_FAST,
  NAV_PREV_MEASURE,
  NAV_SPACE,
  NAV_ZOOM_IN,
  NAV_ZOOM_OUT,
  NAV_ZOOM_IMAGE,
  NONE
};

enum class OPTION {
  TRACK_DIVISION_LIVE,
  TRACK_DIVISION_MIDI,
  SET_HAND_RANGE,
  HAND_RANGE,
  SET_DARKEN_IMAGE,
  DARKEN_IMAGE,
  SET_CIE_FUNCTION,
  CIE_FUNCTION,
  DYNAMIC_LABEL,
  PARTICLE,
  SCALE_VELOCITY,
  SHADOW,
  SHADOW_DISTANCE,
  LIMIT_FPS,
  NONE
};

enum class DIA_OPT { CHECK_ONLY, SLIDER, SUBBOX, NONE };

enum class langType {
  LANG_EN,
  LANG_NONE,
};

enum class PREF { P1, P2, NONE };

enum class DIALOG { PREFERENCES = 0, FILE = 1, INFO = 2, NONE = -1 };

enum panelType {

  MENU_INACTIVE = -1,

  FILE_MENU_FILE = 0,
  FILE_MENU_OPEN_FILE = 1,
  FILE_MENU_OPEN_IMAGE = 2,
  FILE_MENU_SAVE = 3,
  FILE_MENU_SAVE_AS = 4,
  FILE_MENU_CLOSE_FILE = 5,
  FILE_MENU_CLOSE_IMAGE = 6,
  FILE_MENU_RELOAD = 7,
  FILE_MENU_EXIT = 8,

  EDIT_MENU_EDIT = 0,
  EDIT_MENU_SHEET_MUSIC = 1,
  EDIT_MENU_PREFERENCES = 2,

  VIEW_MENU_VIEW = 0,
  VIEW_MENU_DISPLAY_MODE = 1,
  VIEW_MENU_SONG_TIME = 2,
  VIEW_MENU_KEY_SIGNATURE = 3,
  VIEW_MENU_TEMPO = 4,
  VIEW_MENU_NOW_LINE = 5,
  VIEW_MENU_MEASURE_LINE = 6,
  VIEW_MENU_MEASURE_NUMBER = 7,
  VIEW_MENU_BACKGROUND = 8,
  VIEW_MENU_FPS = 9,

  DISPLAY_MENU_DEFAULT = 0,
  DISPLAY_MENU_LINE = 1,
  DISPLAY_MENU_PULSE = 2,
  DISPLAY_MENU_BALL = 3,
  DISPLAY_MENU_FFT = 4,
  DISPLAY_MENU_VORONOI = 5,
  DISPLAY_MENU_LOOP = 6,

  SONG_MENU_RELATIVE = 0,
  SONG_MENU_ABSOLUTE = 1,

  MIDI_MENU_MIDI = 0,
  MIDI_MENU_INPUT = 1,
  MIDI_MENU_OUTPUT = 2,
  MIDI_MENU_LIVE_PLAY = 3,

  COLOR_MENU_COLOR = 0,
  COLOR_MENU_COLOR_BY = 1,
  COLOR_MENU_COLOR_SCHEME = 2,
  COLOR_MENU_SWAP_COLORS = 3,
  COLOR_MENU_INVERT_COLOR_SCHEME = 4,

  SCHEME_MENU_PART = 0,
  SCHEME_MENU_VELOCITY = 1,
  SCHEME_MENU_TONIC = 2,

  INFO_MENU_INFO = 0,
  INFO_MENU_PROGRAM_INFO = 1,
  INFO_MENU_FILE_INFO = 2,
  INFO_MENU_HELP = 3,

  PALETTE_MENU_DEFAULT = 0,
  PALETTE_MENU_FROM_BACKGROUND = 1,
};
