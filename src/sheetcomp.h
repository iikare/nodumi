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

// overlaps up to this long (or half the shorter note) are legato playing, not a second voice
inline constexpr int legato_units = 4;

// notes starting together with different lengths further apart than this many steps are separate voices
inline constexpr int voice_apart = 7;

// a stave with more noteheads than this per quarter note stays in one voice, as two would only add clutter
inline constexpr int voice_max_density = 10;

// each stave carries up to two voices, the upper with stems up and the lower with stems down
inline constexpr int part_count = 4;
inline constexpr int find_part(int stave, int voice) { return stave * 2 + voice; }
inline constexpr int part_stave(int part) { return part / 2; }
inline constexpr int part_voice(int part) { return part % 2; }

// a notehead drawn in this part
inline bool in_part(const sheetNote* n, int part) {
  return n->visible && !n->out_of_bounds && n->stave == part_stave(part) && n->voice == part_voice(part);
}

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

// a written value, an undotted type with up to max_dots dots, each adding half the one before
struct note_value {
  int units;
  int type;
  int dots;
};

inline constexpr int max_dots = 3;

// every value a note piece may take, longest first
inline constexpr note_value note_values[] = {
    {120, NOTE_WHOLE, 3},  {112, NOTE_WHOLE, 2},  {96, NOTE_WHOLE, 1},  {64, NOTE_WHOLE, 0},
    {60, NOTE_HALF, 3},    {56, NOTE_HALF, 2},    {48, NOTE_HALF, 1},   {32, NOTE_HALF, 0},
    {30, NOTE_QUARTER, 3}, {28, NOTE_QUARTER, 2}, {24, NOTE_QUARTER, 1}, {16, NOTE_QUARTER, 0},
    {15, NOTE_8, 3},       {14, NOTE_8, 2},       {12, NOTE_8, 1},      {8, NOTE_8, 0},
    {7, NOTE_16, 2},       {6, NOTE_16, 1},       {4, NOTE_16, 0},      {3, NOTE_32, 1},
    {2, NOTE_32, 0},       {1, NOTE_64, 0},
};

inline int value_units(int type, int dots) { return note_units[type] * 2 - (note_units[type] >> dots); }
inline int note_len(const sheetNote* n) { return value_units(n->display_type, n->dots); }

// the value spanning exactly this many 64ths, with a NOTE_NONE type if there is none
inline note_value find_exact_value(int units) {
  for (const note_value& v : note_values) {
    if (v.units == units) {
      return v;
    }
  }
  return {units, NOTE_NONE, 0};
}

// drawable notes of one chord in one voice of one stave, sharing a stem
struct stem_group {
  int first = -1;  // range of chord indices holding this voice's notes
  int last = -1;
  int lo = 0;  // sheetY extremes
  int hi = 0;
  int stem_lo = INT_MAX;  // sheetY extremes of notes that take a stem
  int stem_hi = INT_MIN;
  int type = -1;  // value the stem shows, the longest that ends before the part's next notes
  int units = 0;  // its length with dots
  int flag_type = FLAGTYPE_NONE;
  int dir = FLAG_UP;
  bool fixed = false;  // the direction comes from the voice, so it is shortened but never turned around
  int beam = -1;
  bool flipped = false;  // a second pushed a notehead to the far side of the stem
  int dots = 0;  // most dots on any of its notes
  int head_width = 0;
  int acc_x = 0;  // distance left of the anchor to the first accidental column
  int dot_x = 0;  // distance right of the anchor to the dots
  int shift = 0;  // distance right of the chord anchor, when a lower voice steps aside from the upper one
  bool limited = false;  // the stem stops short of limit to keep clear of the other stave
  float limit = 0;

  bool empty() const { return first < 0; }
  bool has_stem() const { return flag_type != FLAGTYPE_NONE; }
  bool has_flag() const { return beam < 0 && flag_type >= FLAGTYPE_8 && flag_type <= FLAGTYPE_64; }
};

struct beam_group {
  int stave;
  int voice;
  int dir;
  vector<int> chords;
  bool fixed;    // the direction comes from the voice
  bool limited;  // the outer edge must stay on the near side of limit to keep clear of the other stave
  float limit;

  int part() const { return find_part(stave, voice); }
};

struct sheetChordData {
  int leftWidth = 0;
  int rightWidth = 0;
  float ideal = 0;    // natural distance to the next chord or barline, in quarter note widths
  int min_space = 0;  // closest the next chord or barline may come

  stem_group groups[part_count];  // indexed by part
  int rests[part_count] = {-1, -1, -1, -1};  // rest value per part, if one starts here
  bool rest_shared[2] = {false, false};  // both voices of a stave rest together, drawn once in the middle
  int rest_shift[part_count] = {0, 0, 0, 0};  // how far each rest moves out of the other voice's way, in sheetY steps
  bool oob_high = false;  // notes too high or low for the stave
  bool oob_low = false;
  int arpeggio_x = 0;  // distance left of the anchor to the middle of a rolled chord's wavy line, 0 for none
  int grace_x = 0;     // distance left of the anchor to the grace notes before it, 0 for none

  // the anchor is the left edge of the main notehead column
  int getStemPosition() const { return leftWidth; }
  int getSize() const { return leftWidth + rightWidth; }
};

class sheetMeasure {
 public:
  static int getFlagType(const int noteType);

  static int find_beat(const timeSig& ts);

  void buildChordMap(deque<sheetNote>& vecNote);
  void find_voices(int len);
  void close_gaps(int len, bool open);
  void find_rests(int len, bool open);
  void build_stem_groups(int len);
  void build_beams();
  void set_stem_dirs();
  void place_heads();

  void setParent(measureController& m) { measure = &m; }

  vector<sheetChordData> s_chordData;
  vector<pair<int, vector<sheetNote*>>> chords;
  vector<sheetNote*> graces;  // in time order, each before the chord at its position
  vector<beam_group> beams;
  measureController* measure;

  float rest_ideal = 0;  // natural width of a measure with no notes, in quarter note widths
  bool bar_rest[2] = {false, false};  // a stave silent for the whole measure
  bool two_voice[2] = {false, false};  // a stave whose notes overlap, written as two voices
  bool one_voice[2] = {false, false};  // a stave kept to one voice because two ran into the other stave
  float voice_gap[2] = {0, 0};         // mean pitch distance between the voices, so how far one voice would stretch
  int octave_out[2] = {0, 0};  // a stave's 8va (treble) or 8vb (bass) line runs on into the next measure

 private:
  bool is_plain_half_bar(int part, int half) const;
};
