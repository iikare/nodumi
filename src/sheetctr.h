#pragma once

#include <stdint.h>

#include <algorithm>
#include <array>
#include <vector>

#include "color.h"
#include "log.h"
#include "measure.h"
#include "note.h"
#include "sheetcomp.h"
#include "timekey.h"

using std::make_pair;
using std::pair;
using std::vector;

class midi;

class sheetController {
 public:
  void reset() {
    sheetPageSeparator.clear();
    live_separator.clear();
    displayMeasure.clear();
  }

  int getGlyphWidth(int codepoint, int size = fSize);
  unsigned int getDisplayMeasureCount() const { return displayMeasure.size(); }

  void link_ties(midi& stream);
  void disectMeasure(measureController& measure);
  void popMeasure() { displayMeasure.pop_back(); }
  void findSheetPages();

  void drawSheet();
  void drawSheetPage();

  friend class midi;
  friend class sheetMeasure;

 private:
  struct beam_line {
    float x0;
    float y0;  // outer edge of the primary beam at x0
    float slope;
  };

  // anything above the treble stave that a measure number must clear
  struct label_obstacle {
    float x0;
    float x1;
    float top;
    float bottom;
  };

  struct measure_label {
    int number;
    float x;
    float barline;
    bool first;
  };

  struct drawn_tie {
    float x0;
    float x1;
    float y;
    bool up;
  };

  struct open_tie {
    int note;
    float x;
    float y;
    bool up;
    bool outside;
    float flip_y;   // where the tie would start on the other side of the note
    int head_note;  // the note whose head the tie starts from
  };

  struct head_box {
    float x0;
    float x1;
    float y0;
    float y1;
    int note;
  };

  vector<int> sheetPageSeparator;
  vector<int> live_separator;  // page breaks already settled while playing live
  vector<sheetMeasure> displayMeasure;

  // per-frame scratch space, kept to avoid reallocating every frame
  vector<int> chord_x;
  vector<float> chord_top;
  vector<float> chord_bottom;
  vector<beam_line> beam_lines;
  vector<open_tie> open_ties;
  vector<drawn_tie> drawn_ties;
  vector<head_box> head_boxes;
  vector<label_obstacle> label_obstacles;
  vector<measure_label> measure_labels;
  float staff_end = 0;

  int to_units(const midi& stream, int ticks) const;
  bool is_quantized() const;
  int find_grid(const midi& stream) const;
  int find_note_begin(const midi& stream, const measureController& measure, const note& n) const;
  int find_note_end(const midi& stream, const note& n) const;
  int find_value_units(const midi& stream, const note& n) const;
  int find_raw_value_units(const midi& stream, const note& n) const;
  void build_display_notes(measureController& measure);
  sheetMeasure layout_measure(measureController& measure, const std::array<bool, 2>& one_voice, int (&crowded)[2]);
  void place_rests(sheetMeasure& dm);
  bool clears_other_voice(const sheetMeasure& dm, int ch, int part, int dir, int lo, int hi, float len);
  void keep_in_sheet(sheetMeasure& dm);
  void separate_staves(sheetMeasure& dm, int (&crowded)[2]);
  void avoid_ties(sheetMeasure& dm);
  float find_sheet_top() const;
  float find_sheet_bottom() const;
  void find_chord_widths(sheetMeasure& dm);
  void find_spacing(sheetMeasure& dm);
  int find_measure_fixed(int measureNum, bool includeSig);
  float find_content_width(const sheetMeasure& dm, float scale);

  void draw_measure(const sheetMeasure& dm, int measure_x, int measure_end);
  void add_obstacle(float x0, float x1, float top, float bottom);
  float find_label_y(float x, float width, float ink_top, float ink_bottom, float y) const;
  void place_measure_labels();
  void draw_rest(int type, int stave, float x, int shift = 0);
  int find_rest_y(int type, int stave, int shift = 0);
  float find_rest_extent(int type, bool below);
  void draw_oob_markers(const sheetMeasure& dm, int ch);
  int get_rest_symbol(const int noteType);
  int find_acc_height(const int symbol);
  void draw_group_shapes(const sheetMeasure& dm, int ch, int part, int measure_x);
  void draw_group_glyphs(const sheetMeasure& dm, int ch, int part);
  int find_head_x(const stem_group& g, const sheetNote* n, int anchor);
  int find_flag_y(const stem_group& g, int stave);
  int find_dot_x(const sheetMeasure& dm, int ch, int part, const sheetNote* n);
  int find_dot_row(const sheetMeasure& dm, int part, int sheet_y) const;
  void draw_ledger_lines(const sheetMeasure& dm, int ch, int part, int anchor);
  void draw_beam(const sheetMeasure& dm, int b);
  void draw_tie(float x0, float x1, float y, bool up, float flip_y, int start_note, int end_note);
  float find_tie_arc(float t, float shoulder) const;
  bool tie_hits_heads(float x0, float x1, float y, bool up, float height, float shoulder, float thickness,
                      int start_note, int end_note) const;
  beam_line find_beam_line(const sheetMeasure& dm, const beam_group& beam);
  float find_stem_x(int anchor, int dir) const;
  int stem_anchor(const sheetMeasure& dm, const beam_group& beam, int ch) const;
  void draw_flags(float x, float tip, int count, bool down);

  void drawTimeSignature(const timeSig& time, int x);
  void drawKeySignature(const keySig& key, int x);

  int getKeyWidth(const keySig& key);
  int getTimeWidth(const timeSig& time);
  int getSymbolWidth(const int symbol);
  int getSymbolWidth(const int flagType, const int dir);
  int getSymbolType(const int noteType);
  int getAccType(const int accType);

  void findDFAStartVector(vector<int>& DFAState, const keySig& ks);
  void findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType);
  int findTimePartWidth(const int part);
  int findStaveY(int sheetY, int stave);
  float findMeasureWidth(int measureNum, bool includeSig = false, float scale = natural_scale);
  pair<int, int> findSheetPageLimit(int measureNum) const;

  static constexpr pair<int, int> getStaveRenderLimit() {
    return make_pair(staveFlagLimitMap[0][0], staveFlagLimitMap[1][0]);
  }
  static constexpr int getFlagLimit(int flagType, int stave) {
    return staveFlagLimitMap[stave][flagType == FLAGTYPE_NONE ? 0 : (flagType + 1) % 6];
  }
  int getDisplayAccType(int& DFAState, int noteAccType);
  int mapSheetY(int sheetY);

  static constexpr int sharpHash[7] = {1, 4, 0, 3, 6, 2, 5};
  static constexpr int sharpSpacing[7] = {0, 1, 0, 2, 4, 4, 5};
  static constexpr int sharpY[7] = {-1, -1, -2, -2, 0, -1, -1};

  static constexpr int flatHash[7] = {5, 2, 6, 3, 7, 4, 8};
  static constexpr int flatSpacing[7] = {0, 0, 0, 1, 1, 2, 2};
  static constexpr int flatY[7] = {0, -1, 0, -2, 0, -1, 0};

  // padding for thin "1" doesnt' work on second "1" (i.e. 11, 21, etc)
  // or singular "1"
  static constexpr int timeWidths[10] = {16, 14, 15, 15, 18, 15, 16, 15, 16, 16};

  static constexpr int sharpWidths[8] = {0, 10, 21, 30, 42, 54, 64, 75};
  static constexpr int flatWidths[8] = {0, 8, 18, 28, 39, 49, 60, 70};
  static constexpr int fSize = 157;
  static constexpr int accConstSpacing = 10;
  static constexpr int accMax = 7;

  static constexpr int borderSpacing = 15;

  static constexpr int oobWidth = 16;

  static constexpr int stemWidth = 2;
  static constexpr int dotSpacing = 5;
  static constexpr int accSpacing = 4;
  static constexpr int sigSpacing = 4;
  static constexpr int endSpacing = 4;

  static constexpr int placeholderWidth = 24;

  // engraving metrics, measured from leland at fSize (one staff space is ~9.75px)
  static constexpr int glyph_baseline = 78;  // font ascent, so glyph origins land on y
  static constexpr float head_width_exact = 12.68f;
  static constexpr float stem_attach = 1.6f;  // stems start slightly past the notehead center
  static constexpr int stem_len = 34;
  static constexpr int stem_min_len = 24;  // how short a voice's stem may get to keep clear of the other stave
  static constexpr int flag_extra[4] = {0, 4, 11, 17};  // extra stem for 8th to 64th flags
  static constexpr float flag_step = 6.5f;               // between stacked flags
  static constexpr float flag_scale_single = 0.75f;      // flag tails end about a space above the note to clear dots
  static constexpr float flag_scale_stacked = 0.62f;
  static constexpr int beam_stem_len = 30;  // nearest notehead to the outer beam edge
  static constexpr int beam_min_len = 16;   // how short a beamed stem may get to keep clear of the other stave
  static constexpr float beam_thickness = 4.0f;
  static constexpr float beam_step = 6.5f;
  static constexpr float beam_max_slope = 0.25f;
  static constexpr float beam_max_rise = 10.0f;
  static constexpr float beamlet_width = 10.0f;
  static constexpr int ledger_ext = 3;
  static constexpr int acc_col_width = 11;
  static constexpr int acc_col_max = 4;
  static constexpr int acc_clearance = 6;  // sheetY distance before accidentals can share a column
  static constexpr float head_half = 6.0f;
  static constexpr int acc_above = 18;  // flats reach furthest above their note
  static constexpr int acc_below = 13;
  static constexpr float clef_x = 40.0f;  // treble clef as placed by drawSheet
  static constexpr float clef_width = 25.0f;
  static constexpr float clef_height = 43.0f;  // above the g line
  static constexpr float marker_width = 9.0f;
  static constexpr float marker_height = 6.0f;
  static constexpr float marker_gap = 3.0f;
  static constexpr float sheet_margin = 2.0f;  // stems turning around must stay this far inside the sheet
  static constexpr float label_pad = 2.0f;
  static constexpr int label_size = 14;
  static constexpr float label_slide = 12.0f;       // furthest a measure number moves right of its spot
  static constexpr float label_slide_back = 12.0f;  // extra room over the signatures on a page's first measure
  static constexpr int tie_offset = 4;
  static constexpr int tie_outside_offset = 7;  // clear of the notehead when tying around it
  static constexpr float tie_min_height = 3.0f;
  static constexpr float tie_max_height = 12.0f;
  static constexpr float tie_min_len = 8.0f;
  static constexpr float tie_merge_gap = 24.0f;      // ties into the same head starting this close are drawn once
  static constexpr float tie_head_clearance = 2.0f;  // room a tie keeps from noteheads it passes
  static constexpr float tie_end_width = 0.5f;
  static constexpr int tie_tolerance = 2;  // overhang in 64ths that is clipped instead of tied
  static constexpr double value_tolerance = 0.125;  // relative error allowed when snapping to one value
  static constexpr float natural_scale = 26.0f;  // width of a quarter note when nothing is stretched
  static constexpr float max_scale = natural_scale * 1.75f;  // stretch limit, so short pages stay ragged
  static constexpr float lead_pad = 10.0f;  // after the barline, before the first chord
  static constexpr int trail_pad = 6;  // after the last chord, before the barline
  static constexpr int chord_gap = 3;  // between the extents of neighbouring chords
  static constexpr int min_unit = 4;  // shortest gap that sets a measure's spacing, in 64ths
  static constexpr int live_unit = 8;  // fixed spacing reference for live measures, in 64ths
  static constexpr float stave_clearance = 6.0f;      // closest a treble and bass stem may come between staves
  static constexpr float rest_clearance = 3.0f;  // between a voice's rest and the other voice's noteheads
  static constexpr int rest_shift_max = 6;       // furthest a rest moves past the other voice, in spaces
  static constexpr int voice_cross_gap = 4;      // extra room between the stems of crossing voices

  static constexpr bool debug_layout = false;

  // order: 0, 4, 8, 16, 32, 64
  // limit is the sheetY that causes clipping when facing up
  // first value is the overall limit (above/below will not be rendered)
  static constexpr int staveFlagLimitMap[2][6] = {
      {22, 18, 18, 18, 17, 15},
      {-22, -18, -18, -18, -15, -15},
  };

  // indexed to middle C (val:60, MIDC_NOTE_IDX)
  static constexpr int staveKeySigMap[15][7] = {
      {
          // KEYSIG_C
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_CSHARP
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
      },
      {
          // KEYSIG_DFLAT
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_D
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_EFLAT
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_E
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_F
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_FSHARP
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_GFLAT
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_G
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_AFLAT
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_A
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_BFLAT
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_CLEAR,
          DA_STATE_FLAT,
      },
      {
          // KEYSIG_B
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_SHARP,
          DA_STATE_CLEAR,
      },
      {
          // KEYSIG_CFLAT
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
          DA_STATE_FLAT,
      },
  };
  static constexpr int staveKeySigMapLen = 7;
};
