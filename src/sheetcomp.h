#pragma once

#include <climits>
#include <deque>
#include <vector>

#include "enum.h"
#include "measure.h"
#include "note.h"
#include "sheetnote.h"

using std::deque;
using std::pair;
using std::vector;

// note value lengths in 64ths, indexed by noteType
inline constexpr int note_units[NOTE_NONE] = {128, 96, 64, 48, 32, 24, 16, 12, 8, 6, 4, 2, 1};

// shorter silences are left without a rest so performances stay readable
inline constexpr int rest_min_units = 4;

inline bool is_dotted_type(int type) {
  return type == NOTE_WHOLE_DOT || type == NOTE_HALF_DOT || type == NOTE_QUARTER_DOT || type == NOTE_8_DOT ||
         type == NOTE_16_DOT;
}

// the note value spanning exactly this many 64ths, or NOTE_NONE
inline int find_exact_type(int units) {
  for (int type = NOTE_WHOLE_DOT; type <= NOTE_64; ++type) {
    if (note_units[type] == units) {
      return type;
    }
  }
  return NOTE_NONE;
}

// drawable notes of one chord on one stave, sharing a stem
struct stem_group {
  int first = -1;  // range of chord indices holding this stave's notes
  int last = -1;
  int lo = 0;  // sheetY extremes
  int hi = 0;
  int stem_lo = INT_MAX;  // sheetY extremes of notes that take a stem
  int stem_hi = INT_MIN;
  int type = -1;  // shortest piece in the group
  int flag_type = FLAGTYPE_NONE;
  int dir = FLAG_UP;
  int pref = FLAG_NONE;  // direction away from notes sounding around this one, as a second voice would point
  int beam = -1;
  bool flipped = false;  // a second pushed a notehead to the far side of the stem
  bool dotted = false;
  int head_width = 0;
  int acc_x = 0;  // distance left of the anchor to the first accidental column
  int dot_x = 0;  // distance right of the anchor to the dots

  bool empty() const { return first < 0; }
  bool has_stem() const { return flag_type != FLAGTYPE_NONE; }
  bool has_flag() const { return beam < 0 && flag_type >= FLAGTYPE_8 && flag_type <= FLAGTYPE_64; }
};

struct beam_group {
  int stave;
  int dir;
  vector<int> chords;
  bool limited;  // the outer edge must stay on the near side of limit to keep clear of the other stave
  float limit;
};

struct sheetChordData {
  int leftWidth = 0;
  int rightWidth = 0;
  float ideal = 0;    // natural distance to the next chord or barline, in quarter note widths
  int min_space = 0;  // closest the next chord or barline may come

  stem_group groups[2];  // indexed by stave
  int rests[2] = {-1, -1};  // rest value per stave, if one starts here
  bool oob_high = false;  // notes too high or low for the stave
  bool oob_low = false;

  // the anchor is the left edge of the main notehead column
  int getStemPosition() const { return leftWidth; }
  int getSize() const { return leftWidth + rightWidth; }
};

class sheetMeasure {
 public:
  static int getFlagType(const int noteType);

  void buildChordMap(deque<sheetNote>& vecNote);
  void find_rests(int len, bool open);
  void build_stem_groups();
  void build_beams();
  void find_stem_prefs();
  void set_stem_dirs();
  void place_heads();

  void setParent(measureController& m) { measure = &m; }

  vector<sheetChordData> s_chordData;
  vector<pair<int, vector<sheetNote*>>> chords;
  vector<beam_group> beams;
  measureController* measure;

  float rest_ideal = 0;  // natural width of a measure with no notes, in quarter note widths
  bool bar_rest[2] = {false, false};  // a stave silent for the whole measure

 private:
  bool is_plain_half_bar(int stave, int half) const;
};
