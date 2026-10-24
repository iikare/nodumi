#include "sheetctr.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numeric>

#include "define.h"
#include "enum.h"
#include "wrap.h"

int sheetController::getGlyphWidth(int codepoint, int size) {
  return GetGlyphInfo(ctr.getFont("LELAND", size), codepoint).image.width;
}

void sheetController::drawTimeSignature(const timeSig& time, int x) {
  // y-coordinate is constant in respect to sheet controller parameters, not a
  // user-callable parameter
  const int y = ctr.barMargin + ctr.barWidth * 2;
  const colorRGB col = ctr.bgSheetNote;

  // for centering
  if constexpr (debug_layout) {
    drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255, 0, 0});
  }

  if (time.getTop() < 0 || time.getTop() > 99 || time.getBottom() < 0 || time.getBottom() > 99) {
    logW(LL_WARN, "complex time signature detected with meter", time.getTop(), "/", time.getBottom());
    return;
  }

  x = x + getTimeWidth(timeSig(time.getTop(), time.getBottom(), 0)) / 2;

  // the given x-coordinate is the spatial center of the time signature
  // the given y-coordinate is the top line of the upper staff

  auto drawSingleSig = [&](int val, int yOffset) {
    const int point = SYM_TIME_0 + val;
    const int halfDist = (getGlyphWidth(point) + 1) / 2;

    drawSymbol(point, fSize, x - halfDist, y + yOffset, col);
  };
  auto drawDoubleSig = [&](int val, int yOffset) {
    int padding = 0;

    int numL = val / 10;
    int numR = val % 10;

    if (numL == 1) {
      padding = -2;
    }

    const int pointL = SYM_TIME_0 + numL;
    const int widthL = getGlyphWidth(pointL);
    const int pointR = SYM_TIME_0 + numR;
    const int widthR = getGlyphWidth(pointR);

    const int leftPos = x - (padding + widthL + widthR) / 2;
    const int rightPos = leftPos + widthL + padding;

    drawSymbol(pointL, fSize, leftPos, y + yOffset, col);

    drawSymbol(pointR, fSize, rightPos, y + yOffset, col);
  };

  if (time.getTop() > 9) {  // two-digit top
    drawDoubleSig(time.getTop(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin - ctr.barWidth * 2 + 1);
    drawDoubleSig(time.getTop(),
                  (ctr.sheetHeight - 200) / 2 - ctr.barMargin + ctr.barSpacing - ctr.barWidth * 2 + 1);
  }
  else {
    drawSingleSig(time.getTop(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin - ctr.barWidth * 2 + 1);
    drawSingleSig(time.getTop(),
                  (ctr.sheetHeight - 200) / 2 - ctr.barMargin + ctr.barSpacing - ctr.barWidth * 2 + 1);
  }

  if (time.getBottom() > 9) {  // two-digit bottom
    drawDoubleSig(time.getBottom(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin + 1);
    drawDoubleSig(time.getBottom(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin + ctr.barSpacing + 1);
  }
  else {
    drawSingleSig(time.getBottom(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin + 1);
    drawSingleSig(time.getBottom(), (ctr.sheetHeight - 200) / 2 - ctr.barMargin + ctr.barSpacing + 1);
  }
}

void sheetController::drawKeySignature(const keySig& key, int x) {
  const int y = ctr.barMargin + ctr.barWidth * 2;
  const colorRGB col = ctr.bgSheetNote;

  // for centering
  if constexpr (debug_layout) {
    drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255, 0, 0});
  }

  int symbol = SYM_NONE;
  int prevAcc = 0;
  int prevType = SYM_ACC_NATURAL;
  findKeyData(key, symbol, prevAcc, prevType);

  auto drawKeySigPart = [&](const int symbol, const int index, const int prevType) {
    if (index > accMax || index < 0) {
      return;
    }

    int* accHash = const_cast<int*>(flatHash);
    int* accSpacing = const_cast<int*>(flatSpacing);
    int* accY = const_cast<int*>(flatY);

    double posMod = 0;

    switch (symbol) {
      case SYM_ACC_SHARP:
        accHash = const_cast<int*>(sharpHash);
        accSpacing = const_cast<int*>(sharpSpacing);
        accY = const_cast<int*>(sharpY);
        posMod = accHash[index] - 1;
        break;
      case SYM_ACC_FLAT:
        accHash = const_cast<int*>(flatHash);
        accSpacing = const_cast<int*>(flatSpacing);
        accY = const_cast<int*>(flatY);
        posMod = accHash[index] - 1;
        break;
      case SYM_ACC_NATURAL:
        switch (prevType) {
          case SYM_ACC_SHARP:
            accHash = const_cast<int*>(sharpHash);
            accSpacing = const_cast<int*>(sharpSpacing);
            accY = const_cast<int*>(sharpY);
            posMod = accHash[index] - 1;
            break;
          case SYM_ACC_FLAT:
            accHash = const_cast<int*>(flatHash);
            accSpacing = const_cast<int*>(flatSpacing);
            accY = const_cast<int*>(flatY);
            posMod = accHash[index] - 1;
            break;
        }
        break;
    }
    // logQ(getGlyphWidth(symbol) + index*accConstSpacing+accSpacing[index]);

    const int key_x = x + index * accConstSpacing + accSpacing[index];
    const float treble_y = y - ctr.barSpacing + posMod * (ctr.barWidth / 2.0f + 0.4) - accY[index];
    drawSymbol(symbol, fSize, key_x, treble_y, col);
    add_obstacle(key_x, key_x + getSymbolWidth(symbol), treble_y + glyph_baseline - find_acc_height(symbol),
                 treble_y + glyph_baseline + acc_below);
    drawSymbol(symbol, fSize, x + index * accConstSpacing + accSpacing[index],
               y + posMod * (ctr.barWidth / 2.0f + 0.4) - accY[index] + 1 + ctr.barWidth, col);
  };

  int drawLimit = symbol == SYM_ACC_NATURAL ? prevAcc : key.getSize();

  for (auto i = 0; i < drawLimit; i++) {
    drawKeySigPart(symbol, i, prevType);
  }
}

int sheetController::findStaveY(int sheetY, int stave) {
  if (stave == STAVE_TREBLE) {
    return ctr.barMargin + ctr.barWidth * 7 - ctr.barWidth / 2 * sheetY;
  }
  else {
    return ctr.barMargin + ctr.barSpacing + ctr.barWidth - ctr.barWidth / 2 * sheetY;
  }
}

int sheetController::getKeyWidth(const keySig& key) {
  int symbol = SYM_NONE;
  int prevAcc = 0;
  int prevType = SYM_ACC_NATURAL;
  findKeyData(key, symbol, prevAcc, prevType);

  // drawLine(x,0,x,ctr.getHeight(),ctr.bgNow);

  int xBound = 0;
  switch (symbol) {
    case SYM_ACC_SHARP:
      xBound += sharpWidths[key.getSize()];
      break;
    case SYM_ACC_FLAT:
      xBound += flatWidths[key.getSize()];
      break;
    case SYM_ACC_NATURAL:
      // adjustments are due to difference in width between the accidental
      // symbols
      switch (prevType) {
        case SYM_ACC_SHARP:
          xBound += sharpWidths[prevAcc] - 3;
          break;
        case SYM_ACC_FLAT:
          xBound += flatWidths[prevAcc] - 1;
          break;
      }
      break;
  }

  // drawLine(x+xBound,0,x+xBound,ctr.getHeight(),ctr.bgSheetNote);

  return xBound;
}

int sheetController::getTimeWidth(const timeSig& time) {
  return max(findTimePartWidth(time.getTop()), findTimePartWidth(time.getBottom()));
}

int sheetController::findTimePartWidth(const int part) {
  // error checking already done earlier
  if (part < 10) {
    return timeWidths[part];
  }

  // account for shortening of "1" glyph in range 10-19
  if (part / 10 == 1) {
    return timeWidths[part / 10] - 2 + timeWidths[part % 10];
  }

  return timeWidths[part / 10] + timeWidths[part % 10];
}

void sheetController::findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType) {
  symbol = SYM_NONE;
  prevAcc = 0;
  prevType = SYM_NONE;

  if (key.getAcc() == 0) {
    // C major / A minor
    // find number of courtesy accidentals
    symbol = SYM_ACC_NATURAL;
    if (key.getPrev() != nullptr) {
      prevAcc = key.getPrev()->getSize();
    }
    if (prevAcc != 0) {
      prevType = key.getPrev()->isSharp() ? SYM_ACC_SHARP : SYM_ACC_FLAT;
    }
  }
  else if (key.isSharp()) {
    symbol = SYM_ACC_SHARP;
  }
  else {
    symbol = SYM_ACC_FLAT;
  }
}

namespace {

// splits [begin, end) in 64ths into notatable values, keeping split pieces aligned to their own length
template <typename F>
void split_duration(int begin, int end, F&& emit) {
  int pos = begin;
  while (pos < end) {
    const int remain = end - pos;
    const int exact = find_exact_type(remain);
    if (exact != NOTE_NONE) {
      emit(pos, exact, true);
      return;
    }

    int value = note_units[NOTE_WHOLE];
    while (value > 1 && (value > remain || pos % value != 0)) {
      value /= 2;
    }
    emit(pos, find_exact_type(value), false);
    pos += value;
  }
}

// leland's eighth flag (stem up) traced from the font, in px from the stem tip, tail widened to about 1.7px
constexpr int flag_trace_len = 11;
constexpr float flag_outer[flag_trace_len][2] = {
    {1.0f, -0.3f}, {1.4f, 1.4f},   {2.4f, 3.8f},   {3.6f, 6.0f},   {5.8f, 9.2f},  {8.2f, 12.4f},
    {10.2f, 15.5f}, {11.3f, 19.5f}, {11.1f, 24.0f}, {10.2f, 28.0f}, {8.9f, 31.4f},
};
constexpr float flag_inner[flag_trace_len][2] = {
    {0.0f, 8.9f},  {0.3f, 9.3f},  {2.0f, 9.8f},  {4.0f, 11.0f}, {5.9f, 13.0f}, {7.9f, 16.3f},
    {8.9f, 18.6f}, {9.3f, 21.3f}, {9.1f, 24.6f}, {8.4f, 28.0f}, {7.6f, 30.8f},
};

bool has_acc(const sheetNote* n) {
  return n->displayAcc == ACC_SHARP || n->displayAcc == ACC_FLAT || n->displayAcc == ACC_NATURAL;
}

}  // namespace

int sheetController::to_units(const midi& stream, int ticks) const {
  return static_cast<int>(std::lround(ticks * 16.0 / std::max(1, stream.getTPQ())));
}

int sheetController::find_note_begin(const midi& stream, const measureController& measure, const note& n) const {
  // snap to the nearest grid position, live input on a coarser grid unless one is chosen in the preferences
  int grid = std::max(1, to_units(stream, ctr.getMinTickLen()));
  if (ctr.getLiveState() && ctr.option.get(OPTION::SET_LIVE_QUANTIZE)) {
    grid = ctr.option.get(OPTION::LIVE_QUANTIZE);
  }
  const int len = std::max(1, to_units(stream, measure.getTickLen()));
  const double units = (n.tick - measure.getTick()) * 16.0 / std::max(1, stream.getTPQ());
  const int pos = static_cast<int>(std::lround(units / grid)) * grid;
  return std::clamp(pos, 0, (len - 1) / grid * grid);
}

int sheetController::find_value_units(const midi& stream, const note& n) const {
  const double actual = n.tickDuration * 16.0 / std::max(1, stream.getTPQ());

  // very short notes keep their rounded-up value
  if (actual <= note_units[NOTE_16_DOT]) {
    return note_units[std::clamp(n.type, static_cast<int>(NOTE_16_DOT), static_cast<int>(NOTE_64))];
  }

  // a close single value absorbs shortened or overlapping performed notes
  int best = 0;
  double best_err = value_tolerance;
  for (int type = NOTE_WHOLE_DOT; type <= NOTE_16_DOT; ++type) {
    const double err = std::abs(note_units[type] - actual) / actual;
    if (err <= best_err) {
      best = note_units[type];
      best_err = err;
    }
  }
  if (best) {
    return best;
  }

  // otherwise keep the real length so it splits into tied values
  const int quarter = note_units[NOTE_QUARTER];
  if (actual > note_units[NOTE_WHOLE_DOT]) {
    return static_cast<int>(std::ceil(actual / quarter)) * quarter;
  }
  const int sixteenth = note_units[NOTE_16];
  return std::max(sixteenth, static_cast<int>(std::lround(actual / sixteenth)) * sixteenth);
}

int sheetController::find_note_end(const midi& stream, const note& n) const {
  if (n.measure < 0 || n.measure >= static_cast<int>(stream.measureMap.size())) {
    return 0;
  }
  const measureController& measure = stream.measureMap[n.measure];
  return to_units(stream, measure.getTick()) + find_note_begin(stream, measure, n) + find_value_units(stream, n);
}

void sheetController::link_ties(midi& stream) {
  auto& measures = stream.measureMap;
  const int count = measures.size();

  for (auto& m : measures) {
    m.tie_in_notes.clear();
    m.tie_out_notes.clear();
  }

  for (const auto& n : stream.notes) {
    const int idx = n.measure;
    if (idx < 0 || idx + 1 >= count) {
      continue;
    }

    const int end = find_note_end(stream, n);
    const int bar_end = to_units(stream, measures[idx].getTick() + measures[idx].getTickLen());

    // slight overhang from legato playing is clipped instead of tied
    if (end <= bar_end || to_units(stream, n.tick + n.tickDuration) <= bar_end + tie_tolerance) {
      continue;
    }

    measures[idx].tie_out_notes.push_back(n.number);
    for (int k = idx + 1; k < count; ++k) {
      measures[k].tie_in_notes.push_back(n.number);
      if (k + 1 == count || end <= to_units(stream, measures[k].getTick() + measures[k].getTickLen())) {
        break;
      }
      measures[k].tie_out_notes.push_back(n.number);
    }
  }
}

void sheetController::build_display_notes(measureController& measure) {
  const midi& stream = ctr.getStream();
  const int bar_begin = to_units(stream, measure.getTick());
  const int len = std::max(1, to_units(stream, measure.getTickLen()));

  measure.displayNotes.clear();

  auto add_note = [&](int idx, int begin, bool tie_in) {
    const bool tie_out = std::ranges::find(measure.tie_out_notes, idx) != measure.tie_out_notes.end();
    int end = tie_out ? len : std::min(len, find_note_end(stream, stream.notes[idx]) - bar_begin);
    // a held live note may have only just crossed the barline
    end = std::min(len, std::max(end, begin + (tie_in ? note_units[NOTE_16] : 1)));

    split_duration(begin, end, [&](int pos, int type, bool last) {
      measure.displayNotes.push_back({pos, ACC_NONE, STAVE_NONE, true, true, false, idx, type,
                                      tie_in || pos != begin, tie_out || !last, false, 0, false});
    });
  };

  for (int idx : measure.notes) {
    add_note(idx, find_note_begin(stream, measure, stream.notes[idx]), false);
  }
  for (int idx : measure.tie_in_notes) {
    add_note(idx, 0, true);
  }
}

void sheetController::disectMeasure(measureController& measure) {
  // logQ("measure", measure.getNumber(), "has timesig",
  // measure.currentTime.getTop(), measure.currentTime.getBottom());
  //  preprocessing
  const midi& stream = ctr.getStream();

  build_display_notes(measure);

  sheetMeasure dm;
  dm.setParent(measure);
  dm.buildChordMap(measure.displayNotes);

  // run a DFA for each valid midi key, advancing state based on
  // new-note-to-midi-position values each DFA set is unique to its measure add
  // "1" to size for inclusive bounding
  vector<int> presentDFAState;
  findDFAStartVector(presentDFAState, measure.currentKey);

  // find true accidental type for each note
  for (const auto& c : dm.chords) {
    for (unsigned int ct = 0; const auto& n : c.second) {
      const note& src = stream.notes[n->oriNote];

      // mark out-of-bounds notes, which still occupy their stave for rests
      if (src.sheetY > getStaveRenderLimit().first || src.sheetY < getStaveRenderLimit().second) {
        n->out_of_bounds = true;
        n->stave = src.sheetY > 0 ? STAVE_TREBLE : STAVE_BASS;
        continue;
      }

      // tied continuations keep the accidental shown on their first piece
      if (!n->tie_in) {
        n->displayAcc = getDisplayAccType(presentDFAState[mapSheetY(src.sheetY)], src.accType);
      }

      if (src.sheetY > 0) {
        n->stave = STAVE_TREBLE;
      }
      else if (src.sheetY < 0) {
        n->stave = STAVE_BASS;
      }
      else {
        ct >= c.second.size() / 2 ? n->stave = STAVE_TREBLE
                                  : (ct != 0 ? n->stave = STAVE_BASS : n->stave = STAVE_TREBLE);
      }
      ct++;
    }
  }

  // a tie never switches staves, so a continued middle c stays where its note started
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (sheetNote* n : dm.chords[ch].second) {
      const note& src = stream.notes[n->oriNote];
      if (!n->tie_in || n->out_of_bounds || src.sheetY != 0) {
        continue;
      }

      const sheetNote* prev = nullptr;
      for (int earlier = static_cast<int>(ch) - 1; earlier >= 0 && !prev; --earlier) {
        for (const sheetNote* m : dm.chords[earlier].second) {
          if (m->oriNote == n->oriNote) {
            prev = m;
          }
        }
      }
      if (!prev && src.measure >= 0 && src.measure < static_cast<int>(stream.measureMap.size()) &&
          &stream.measureMap[src.measure] != &measure) {
        for (const sheetNote& m : stream.measureMap[src.measure].displayNotes) {
          if (m.oriNote == n->oriNote && !m.tie_in) {
            prev = &m;
          }
        }
      }
      if (prev && prev->stave != STAVE_NONE) {
        n->stave = prev->stave;
      }
    }
  }

  // remove duplicate notes (rendering on same position), which follow the visible one's stave
  for (const auto& c : dm.chords) {
    for (unsigned int ct = 1; ct < c.second.size(); ++ct) {
      if (stream.notes[c.second[ct]->oriNote].sheetY == stream.notes[c.second[ct - 1]->oriNote].sheetY) {
        c.second[ct]->visible = false;
        c.second[ct]->stave = c.second[ct - 1]->stave;
      }
    }
  }

  const bool open = ctr.getLiveState() && &measure == &stream.measureMap.back();
  dm.find_rests(std::max(1, to_units(stream, measure.getTickLen())), open);

  dm.build_stem_groups();
  dm.build_beams();
  dm.find_stem_prefs();
  dm.set_stem_dirs();
  keep_in_sheet(dm);
  separate_staves(dm);
  avoid_ties(dm);
  dm.place_heads();

  find_chord_widths(dm);
  find_spacing(dm);

  displayMeasure.push_back(std::move(dm));
}

float sheetController::find_sheet_top() const { return ctr.menuHeight + sheet_margin; }

float sheetController::find_sheet_bottom() const {
  // the sheet's height is fixed even while it is hidden, so layout does not depend on it being shown
  return ctr.menuHeight + ctr.menuHeight + ctr.sheetHeight - sheet_margin;
}

void sheetController::keep_in_sheet(sheetMeasure& dm) {
  // stems and beams never leave the sheet, so one that would turns around when the other way fits
  auto turn = [&](int dir, int lo, int hi, int stave, float len) {
    const float up_tip = findStaveY(hi, stave) - len;
    const float down_tip = findStaveY(lo, stave) + len;
    if (dir == FLAG_UP && up_tip < find_sheet_top() && down_tip <= find_sheet_bottom()) {
      return static_cast<int>(FLAG_DOWN);
    }
    if (dir == FLAG_DOWN && down_tip > find_sheet_bottom() && up_tip >= find_sheet_top()) {
      return static_cast<int>(FLAG_UP);
    }
    return dir;
  };

  for (auto& b : dm.beams) {
    int lo = std::numeric_limits<int>::max();
    int hi = std::numeric_limits<int>::min();
    int levels = 0;
    for (int ch : b.chords) {
      const stem_group& g = dm.s_chordData[ch].groups[b.stave];
      lo = std::min(lo, g.stem_lo);
      hi = std::max(hi, g.stem_hi);
      levels = std::max(levels, g.flag_type - FLAGTYPE_8);
    }
    b.dir = turn(b.dir, lo, hi, b.stave, beam_stem_len + levels * beam_step + beam_thickness);
    for (int ch : b.chords) {
      dm.s_chordData[ch].groups[b.stave].dir = b.dir;
    }
  }

  for (auto& cd : dm.s_chordData) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& g = cd.groups[stave];
      if (!g.empty() && g.has_stem() && g.beam < 0) {
        const int extra = g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0;
        g.dir = turn(g.dir, g.stem_lo, g.stem_hi, stave, stem_len + extra);
      }
    }
  }
}

void sheetController::separate_staves(sheetMeasure& dm) {
  // a beam or lone stem pointing into the gap between the staves
  struct stem_unit {
    int first;
    int last;
    float tip;          // outer end as it points now
    float flipped_tip;  // outer end if it turned around
    float give;         // how far a beam can pull back towards its notes
    int strength;       // how strongly the notes prefer this direction
    int beam;
    int ch;
    bool done;
  };

  vector<stem_unit> units[2];
  const float sheet_top = find_sheet_top();
  const float sheet_bottom = find_sheet_bottom();

  auto add_unit = [&](int stave, int first, int last, int beam, int ch, const auto& groups) {
    const bool treble = stave == STAVE_TREBLE;
    const int mid = treble ? 6 : -6;
    int lo = std::numeric_limits<int>::max();
    int hi = std::numeric_limits<int>::min();
    int levels = 0;
    for (const stem_group* g : groups) {
      lo = std::min(lo, g->stem_lo);
      hi = std::max(hi, g->stem_hi);
      levels = std::max(levels, g->flag_type - FLAGTYPE_8);
    }
    const float len = beam_stem_len + levels * beam_step + beam_thickness;
    const float up_tip = findStaveY(hi, stave) - len;
    const float down_tip = findStaveY(lo, stave) + len;
    units[stave].push_back({first, last, treble ? down_tip : up_tip, treble ? up_tip : down_tip,
                            beam >= 0 ? static_cast<float>(beam_stem_len - beam_min_len) : 0.0f,
                            std::abs((hi - mid) - (mid - lo)), beam, ch, false});
  };

  for (unsigned int b = 0; b < dm.beams.size(); ++b) {
    const beam_group& beam = dm.beams[b];
    if (beam.dir != (beam.stave == STAVE_TREBLE ? FLAG_DOWN : FLAG_UP)) {
      continue;
    }
    vector<const stem_group*> groups;
    for (int ch : beam.chords) {
      groups.push_back(&dm.s_chordData[ch].groups[beam.stave]);
    }
    add_unit(beam.stave, beam.chords.front(), beam.chords.back(), b, -1, groups);
  }

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      const stem_group& g = dm.s_chordData[ch].groups[stave];
      if (!g.empty() && g.has_stem() && g.beam < 0 && g.dir == (stave == STAVE_TREBLE ? FLAG_DOWN : FLAG_UP)) {
        add_unit(stave, ch, ch, -1, ch, std::array<const stem_group*, 1>{&g});
      }
    }
  }

  // turning around is only an option when the stems still fit on the sheet
  auto fits_flipped = [&](const stem_unit& u, int stave) {
    return stave == STAVE_TREBLE ? u.flipped_tip >= sheet_top : u.flipped_tip <= sheet_bottom;
  };
  auto flip = [&](stem_unit& u, int stave) {
    const int dir = stave == STAVE_TREBLE ? FLAG_UP : FLAG_DOWN;
    if (u.beam >= 0) {
      dm.beams[u.beam].dir = dir;
      dm.beams[u.beam].limited = false;
      for (int ch : dm.beams[u.beam].chords) {
        dm.s_chordData[ch].groups[stave].dir = dir;
      }
    }
    else {
      dm.s_chordData[u.ch].groups[stave].dir = dir;
    }
    u.done = true;
  };
  auto pull_back = [&](const stem_unit& u, float limit) {
    if (u.beam >= 0) {
      dm.beams[u.beam].limited = true;
      dm.beams[u.beam].limit = limit;
    }
  };

  // when both staves reach into the gap at the same time, the weaker preference gives way
  const float gap_mid = (findStaveY(2, STAVE_TREBLE) + findStaveY(-2, STAVE_BASS)) / 2.0f;
  for (auto& t : units[STAVE_TREBLE]) {
    for (auto& b : units[STAVE_BASS]) {
      if (t.done || b.done || t.first > b.last || b.first > t.last || b.tip - t.tip >= stave_clearance) {
        continue;
      }
      const bool can_flip_t = fits_flipped(t, STAVE_TREBLE);
      const bool can_flip_b = fits_flipped(b, STAVE_BASS);
      if (can_flip_b && (!can_flip_t || t.strength > b.strength)) {
        flip(b, STAVE_BASS);
      }
      else if (can_flip_t) {
        flip(t, STAVE_TREBLE);
      }
      else {
        // neither can turn without leaving the sheet, so both pull back from the middle of the gap
        pull_back(t, gap_mid - stave_clearance / 2);
        pull_back(b, gap_mid + stave_clearance / 2);
      }
    }
  }

  // noteheads, rests and stave lines cannot move, so a stem reaching them pulls back or turns around
  auto resolve = [&](stem_unit& u, int stave, float bound) {
    const bool treble = stave == STAVE_TREBLE;
    const float overlap = treble ? u.tip + stave_clearance - bound : bound - (u.tip - stave_clearance);
    if (overlap <= 0) {
      return;
    }
    const float limit = treble ? bound - stave_clearance : bound + stave_clearance;
    if (overlap <= u.give || !fits_flipped(u, stave)) {
      pull_back(u, limit);
    }
    else {
      flip(u, stave);
    }
  };

  // an outer tie bends away from its chord, so it can reach into the gap as well
  const midi& stream = ctr.getStream();
  auto find_tie_reach = [&](int stave, int first, int last, float bound) {
    const bool treble = stave == STAVE_TREBLE;
    for (int c = 0; c <= last && c < static_cast<int>(dm.chords.size()); ++c) {
      const stem_group& g = dm.s_chordData[c].groups[stave];
      if (g.empty()) {
        continue;
      }
      const auto& notes = dm.chords[c].second;
      const sheetNote* n = notes[treble ? g.first : g.last];
      if (!n->tie_in && !n->tie_out) {
        continue;
      }

      // a lone note ties away from its stem, the outer notes of a chord away from the chord
      int count = 0;
      for (int i = g.first; i <= g.last; ++i) {
        count += notes[i]->visible && !notes[i]->out_of_bounds && notes[i]->stave == stave;
      }
      if (count == 1 && g.dir != (treble ? FLAG_UP : FLAG_DOWN)) {
        continue;
      }

      // the tie spans from the measure start or this chord to the next piece of the note
      int start = n->tie_in ? 0 : c;
      int end = c;
      if (n->tie_out) {
        end = dm.chords.size();
        for (int e = c + 1; e < static_cast<int>(dm.chords.size()); ++e) {
          if (std::ranges::find(dm.chords[e].second, n->oriNote, &sheetNote::oriNote) != dm.chords[e].second.end()) {
            end = e;
            break;
          }
        }
      }
      if (end < first || start > last) {
        continue;
      }

      const float y = findStaveY(stream.notes[n->oriNote].sheetY, stave);
      bound = treble ? std::max(bound, y + tie_outside_offset + tie_max_height)
                     : std::min(bound, y - tie_outside_offset - tie_max_height);
    }
    return bound;
  };

  for (auto& b : units[STAVE_BASS]) {
    if (b.done) {
      continue;
    }
    float floor = find_tie_reach(STAVE_TREBLE, b.first, b.last, findStaveY(2, STAVE_TREBLE));
    for (int ch = b.first; ch <= b.last; ++ch) {
      const sheetChordData& cd = dm.s_chordData[ch];
      if (!cd.groups[STAVE_TREBLE].empty()) {
        floor = std::max(floor, findStaveY(cd.groups[STAVE_TREBLE].lo, STAVE_TREBLE) + head_half);
      }
      if (cd.rests[STAVE_TREBLE] >= 0) {
        floor = std::max(floor, find_rest_y(cd.rests[STAVE_TREBLE], STAVE_TREBLE) +
                                    find_rest_extent(cd.rests[STAVE_TREBLE], true));
      }
    }
    resolve(b, STAVE_BASS, floor);
  }
  for (auto& t : units[STAVE_TREBLE]) {
    if (t.done) {
      continue;
    }
    float ceiling = find_tie_reach(STAVE_BASS, t.first, t.last, findStaveY(-2, STAVE_BASS));
    for (int ch = t.first; ch <= t.last; ++ch) {
      const sheetChordData& cd = dm.s_chordData[ch];
      if (!cd.groups[STAVE_BASS].empty()) {
        ceiling = std::min(ceiling, findStaveY(cd.groups[STAVE_BASS].hi, STAVE_BASS) - head_half);
      }
      if (cd.rests[STAVE_BASS] >= 0) {
        ceiling = std::min(ceiling, find_rest_y(cd.rests[STAVE_BASS], STAVE_BASS) -
                                        find_rest_extent(cd.rests[STAVE_BASS], false));
      }
    }
    resolve(t, STAVE_TREBLE, ceiling);
  }
}

void sheetController::avoid_ties(sheetMeasure& dm) {
  // a tie from an earlier chord runs under or over later ones, so stems crossing it turn away instead
  const midi& stream = ctr.getStream();
  const int chord_count = dm.chords.size();

  struct tie_span {
    int start;
    int end;
    float y;  // the tied note, the tie bends away from it
  };

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    // only turning away from the gap between the staves, so the staves stay apart
    const bool treble = stave == STAVE_TREBLE;
    const int toward_gap = treble ? FLAG_DOWN : FLAG_UP;

    // outer ties on the gap side, which is where a stem pointing into the gap would cross them
    vector<tie_span> spans;
    for (int c = 0; c < chord_count; ++c) {
      const stem_group& g = dm.s_chordData[c].groups[stave];
      if (g.empty()) {
        continue;
      }
      const auto& notes = dm.chords[c].second;
      const sheetNote* n = notes[treble ? g.first : g.last];
      if (!n->tie_out) {
        continue;
      }
      int count = 0;
      for (int i = g.first; i <= g.last; ++i) {
        count += notes[i]->visible && !notes[i]->out_of_bounds && notes[i]->stave == stave;
      }
      if (count == 1 && g.dir == toward_gap) {
        continue;
      }
      int end = chord_count;
      for (int e = c + 1; e < chord_count; ++e) {
        if (std::ranges::find(dm.chords[e].second, n->oriNote, &sheetNote::oriNote) != dm.chords[e].second.end()) {
          end = e;
          break;
        }
      }
      spans.push_back({c, end, static_cast<float>(findStaveY(stream.notes[n->oriNote].sheetY, stave))});
    }
    if (spans.empty()) {
      continue;
    }

    // a chord the tie passes under (or over) crosses it when its stemmed notes sit on the tied note's side
    auto crosses = [&](int ch) {
      const stem_group& g = dm.s_chordData[ch].groups[stave];
      const float near = findStaveY(treble ? g.stem_lo : g.stem_hi, stave);
      for (const auto& span : spans) {
        if (span.start < ch && ch < span.end && (treble ? near <= span.y : near >= span.y)) {
          return true;
        }
      }
      return false;
    };
    // a beam may overshoot by as much as its stems can shorten, since it is pulled back inside the sheet when drawn
    auto fits_away = [&](int lo, int hi, float len, float give) {
      return treble ? findStaveY(hi, stave) - len + give >= find_sheet_top()
                    : findStaveY(lo, stave) + len - give <= find_sheet_bottom();
    };
    const int away = treble ? FLAG_UP : FLAG_DOWN;

    for (auto& b : dm.beams) {
      if (b.stave != stave || b.dir != toward_gap) {
        continue;
      }
      int lo = std::numeric_limits<int>::max();
      int hi = std::numeric_limits<int>::min();
      int levels = 0;
      for (int ch : b.chords) {
        const stem_group& g = dm.s_chordData[ch].groups[stave];
        lo = std::min(lo, g.stem_lo);
        hi = std::max(hi, g.stem_hi);
        levels = std::max(levels, g.flag_type - FLAGTYPE_8);
      }
      if (std::ranges::any_of(b.chords, crosses) &&
          fits_away(lo, hi, beam_stem_len + levels * beam_step + beam_thickness, beam_stem_len - beam_min_len)) {
        b.dir = away;
        b.limited = false;
        for (int ch : b.chords) {
          dm.s_chordData[ch].groups[stave].dir = away;
        }
      }
    }

    for (int ch = 0; ch < chord_count; ++ch) {
      stem_group& g = dm.s_chordData[ch].groups[stave];
      if (g.empty() || !g.has_stem() || g.beam >= 0 || g.dir != toward_gap) {
        continue;
      }
      const int extra = g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0;
      if (crosses(ch) && fits_away(g.stem_lo, g.stem_hi, stem_len + extra, 0)) {
        g.dir = away;
      }
    }
  }
}

void sheetController::find_chord_widths(sheetMeasure& dm) {
  const midi& stream = ctr.getStream();
  const int head = getSymbolWidth(SYM_HEAD_STD);

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    sheetChordData& cd = dm.s_chordData[ch];
    const auto& c = dm.chords[ch].second;

    int left = 0;
    int right = 0;

    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& g = cd.groups[stave];
      if (g.empty()) {
        continue;
      }

      // accidentals stack into columns leftwards when they would overlap vertically
      int acc_cols = 0;
      int col_y[acc_col_max] = {};
      g.head_width = 0;
      for (int i = g.last; i >= g.first; --i) {
        sheetNote* n = c[i];
        if (!n->visible || n->out_of_bounds || n->stave != stave) {
          continue;
        }
        g.head_width = std::max(g.head_width, getSymbolWidth(getSymbolType(n->display_type)));
        if (!has_acc(n)) {
          continue;
        }

        const int y = stream.notes[n->oriNote].sheetY;
        int col = 0;
        while (col < acc_cols && col_y[col] - y < acc_clearance) {
          ++col;
        }
        col = std::min(col, acc_col_max - 1);
        acc_cols = std::max(acc_cols, col + 1);
        col_y[col] = y;
        n->acc_col = col;
      }

      int g_left = 0;
      int g_right = g.head_width;
      if (g.flipped && g.dir == FLAG_DOWN) {
        g_left = head;
      }
      else if (g.flipped) {
        g_right = head + g.head_width;
      }
      g.acc_x = g_left + accSpacing;
      if (acc_cols) {
        g_left = g.acc_x + acc_cols * acc_col_width;
      }

      // dots sit right after the noteheads, tucked under an up flag rather than past it
      if (g.dotted) {
        g.dot_x = g_right + dotSpacing;
        g_right = g.dot_x + getSymbolWidth(SYM_DOT);
      }

      if (g.has_flag()) {
        const int flag_w = getSymbolWidth(g.flag_type, g.dir);
        g_right = std::max(g_right, g.dir == FLAG_DOWN ? flag_w : head - stemWidth + flag_w);
      }

      left = std::max(left, g_left);
      right = std::max(right, g_right);
    }

    // rests are centered on the notehead column
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      if (cd.rests[stave] < 0) {
        continue;
      }
      const int rest_w = getSymbolWidth(get_rest_symbol(cd.rests[stave]));
      const int rest_x = (head - rest_w) / 2;
      const int dot_w = is_dotted_type(cd.rests[stave]) ? dotSpacing + getSymbolWidth(SYM_DOT) : 0;
      left = std::max(left, -rest_x);
      right = std::max(right, rest_x + rest_w + dot_w);
    }

    // room for the out-of-bounds marker, centered on the notehead column
    for (const auto* n : c) {
      if (n->out_of_bounds) {
        if (n->stave == STAVE_TREBLE) {
          cd.oob_high = true;
        }
        else {
          cd.oob_low = true;
        }
        left = std::max(left, oobWidth / 2 - head / 2);
        right = std::max(right, head / 2 + oobWidth / 2);
      }
    }

    cd.leftWidth = left;
    cd.rightWidth = right;
  }
}

void sheetController::find_spacing(sheetMeasure& dm) {
  const int len = std::max(1, to_units(ctr.getStream(), dm.measure->getTickLen()));
  const float quarter = note_units[NOTE_QUARTER];

  // space is linear in time within a measure, so equal durations always get equal room,
  // and the shortest gap sets the rate so busy measures get more of it
  int shortest = len;
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    const int next = ch + 1 < dm.chords.size() ? dm.chords[ch + 1].first : len;
    shortest = std::min(shortest, std::max(1, next - dm.chords[ch].first));
  }
  // live measures keep one rate so notes already placed stay put as more arrive
  const int reference = ctr.getLiveState() ? live_unit : std::max(shortest, min_unit);
  const float per_unit = std::sqrt(quarter / reference) / quarter;

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    sheetChordData& cd = dm.s_chordData[ch];
    const bool last = ch + 1 == dm.chords.size();
    const int next = last ? len : dm.chords[ch + 1].first;
    cd.ideal = std::max(1, next - dm.chords[ch].first) * per_unit;
    cd.min_space = cd.rightWidth + (last ? trail_pad : chord_gap + dm.s_chordData[ch + 1].leftWidth);
  }

  dm.rest_ideal = len * per_unit;
}

int sheetController::find_measure_fixed(int measureNum, bool includeSig) {
  sheetMeasure& dm = displayMeasure[measureNum];

  int width = sigSpacing * 2;  // default side spacing

  for (const auto& ts : dm.measure->timeSignatures) {
    width += getTimeWidth(ts);
    width += sigSpacing;
  }
  for (const auto& ks : dm.measure->keySignatures) {
    if (ks.getPrev() != nullptr || ks.getAcc() != 0) {
      width += getKeyWidth(ks);
      width += sigSpacing;
    }
  }

  if (dm.measure->keySignatures.empty() && includeSig) {
    width += getKeyWidth(dm.measure->currentKey);
    width += sigSpacing;
  }

  // double bar line on time/key changes
  if (!dm.measure->keySignatures.empty() || !dm.measure->timeSignatures.empty()) {
    width += sigSpacing;
  }

  // ending bar line
  if (displayMeasure.back().measure->getNumber() == measureNum + 1) {
    if (!ctr.getLiveState()) {
      width += sigSpacing + endSpacing;
    }
  }

  return width;
}

float sheetController::find_content_width(const sheetMeasure& dm, float scale) {
  // a measure holding only tied notes is not empty
  if (dm.chords.empty()) {
    return lead_pad + std::max(dm.rest_ideal * scale, static_cast<float>(getSymbolWidth(SYM_REST_WHOLE)) + lead_pad);
  }

  float width = lead_pad + dm.s_chordData.front().leftWidth;
  for (const auto& cd : dm.s_chordData) {
    width += std::max(cd.ideal * scale, static_cast<float>(cd.min_space));
  }
  return width;
}

float sheetController::findMeasureWidth(int measureNum, bool includeSig, float scale) {
  return find_measure_fixed(measureNum, includeSig) + find_content_width(displayMeasure[measureNum], scale);
}

void sheetController::drawSheet() {
  const midi& stream = ctr.getStream();

  // bg
  drawRectangle(0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight, ctr.bgSheet);

  drawSymbol(SYM_STAFF_BRACE, 480, 17.0f, float(ctr.menuHeight + ctr.barMargin) - 120, ctr.bgSheetNote);
  drawSymbol(SYM_CLEF_TREBLE, 155, 40.0f, ctr.menuHeight + ctr.barMargin - 47, ctr.bgSheetNote);
  drawSymbol(SYM_CLEF_BASS, 155, 40.0f, float(ctr.menuHeight + ctr.barSpacing + ctr.barMargin - 67),
             ctr.bgSheetNote);

  // the page decides where the staves end, so they are drawn after it
  const float full_end = ctr.getWidth() - ctr.sheetSideMargin;
  staff_end = full_end;
  if (stream.measureMap.size() != 0) {
    if (stream.measureMap.size() == stream.sheetData.getDisplayMeasureCount()) {
      drawSheetPage();
    }
  }

  // stave lines
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 5; j++) {
      drawLineEx(ctr.sheetSideMargin, ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth,
                 staff_end, ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth, 1,
                 ctr.bgSheetNote);
    }
  }

  //// end lines
  drawLineEx(ctr.sheetSideMargin, ctr.menuHeight + ctr.barMargin, ctr.sheetSideMargin,
             ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
  if (staff_end == full_end) {
    drawLineEx(full_end, ctr.menuHeight + ctr.barMargin, full_end,
               ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
  }
}

void sheetController::findSheetPages() {
  sheetPageSeparator.clear();
  sheetPageSeparator.push_back(0);

  // pages break on natural widths, then stretch to fill when drawn
  float pageWidth = 0;
  float measureWidth = 0;
  int maxWidth = ctr.getWidth() - ctr.sheetSideMargin - ctr.sheetSymbolWidth;

  if (ctr.getLiveState()) {
    // live pages fill front to back and stay put once full, so only the page being played can change
    sheetPageSeparator = std::move(live_separator);
    if (sheetPageSeparator.empty()) {
      sheetPageSeparator.push_back(0);
    }
    while (sheetPageSeparator.size() > 1 && sheetPageSeparator.back() >= ctr.getMeasureCount()) {
      sheetPageSeparator.pop_back();
    }

    const int page_start = sheetPageSeparator.back();
    for (int i = page_start; i < ctr.getMeasureCount(); ++i) {
      measureWidth = findMeasureWidth(i, i == sheetPageSeparator.back());
      if (pageWidth + measureWidth > maxWidth && i > sheetPageSeparator.back()) {
        sheetPageSeparator.push_back(i);
        pageWidth = 0;
        measureWidth = findMeasureWidth(i, true);  // include current keysig
      }
      pageWidth += measureWidth;
    }
    live_separator = sheetPageSeparator;
  }
  else {
    for (int i = 0; i < ctr.getMeasureCount(); ++i) {
      measureWidth = findMeasureWidth(i);
      // logQ(i+1, measureWidth);
      if (pageWidth + measureWidth > maxWidth) {
        // logQ("divider at", i+1, "extra space", maxWidth - pageWidth);
        sheetPageSeparator.push_back(i);
        pageWidth = 0;
        measureWidth = findMeasureWidth(i, true);  // include current keysig
      }
      pageWidth += measureWidth;
    }
  }
}

pair<int, int> sheetController::findSheetPageLimit(int measureNum) const {
  // return value is inclusive in interval [min, max]
  pair<int, int> result = {0, 0};

  // treat measure 0 as measure 1
  if (measureNum == 0) {
    measureNum = 1;
  }

  if (sheetPageSeparator.empty()) {
    return result;
  }

  if (measureNum > sheetPageSeparator[sheetPageSeparator.size() - 1]) {
    result.first = sheetPageSeparator[sheetPageSeparator.size() - 1] + 1;  // is 0-indexed
    result.second = ctr.getMeasureCount();                                 // is 1-indexed
  }
  else {
    for (unsigned int i = 0; i < sheetPageSeparator.size() - 1; ++i) {
      if (measureNum > sheetPageSeparator[i]) {
        result.first = sheetPageSeparator[i] + 1;
        result.second = sheetPageSeparator[i + 1];
      }
    }
  }

  return result;
}

void sheetController::findDFAStartVector(vector<int>& DFAState, const keySig& ks) {
  DFAState.resize(MAX_STAVE_IDX - MIN_STAVE_IDX + 1);

  const int* keyStateTemplate = staveKeySigMap[ks.getKey()];

  for (unsigned int idx = 0; idx < DFAState.size(); ++idx) {
    int templateIdx = idx + abs(MIN_STAVE_IDX) + (abs(MIN_STAVE_IDX) % 7);
    // mark middle C
    // int state = idx == -MIN_STAVE_IDX ? -1 : keyStateTemplate[(1 +
    // templateIdx) % 7];
    int state = keyStateTemplate[(1 + templateIdx) % 7];
    DFAState[idx] = state;
  }
}

int sheetController::mapSheetY(int sheetY) {
  // input [0, MIN_STAVE_IDX + MAX_STAVE_IDX + 1]
  return sheetY + abs(MIN_STAVE_IDX);
}

void sheetController::drawSheetPage() {
  int offset = ctr.sheetSymbolWidth;
  const midi& stream = ctr.getStream();

  pair<int, int> measureRange = findSheetPageLimit(ctr.getCurrentMeasure());
  if (measureRange.first < 1) {
    return;
  }

  // find the one stretch factor that fills the page, capped so short pages stay ragged
  const float available = ctr.getWidth() - ctr.sheetSideMargin - ctr.sheetSymbolWidth;
  float fixed_width = 0;
  for (int m = measureRange.first; m <= measureRange.second; ++m) {
    fixed_width += find_measure_fixed(m - 1, m == measureRange.first);
  }
  auto page_width = [&](float scale) {
    float width = fixed_width;
    for (int m = measureRange.first; m <= measureRange.second; ++m) {
      width += find_content_width(displayMeasure[m - 1], scale);
    }
    return width;
  };

  // only the last page of a file and live pages stay ragged, the rest are justified edge to edge,
  // and live pages never stretch so nothing moves as notes arrive
  const bool ragged = ctr.getLiveState() || measureRange.second >= static_cast<int>(displayMeasure.size());
  float scale_limit = ctr.getLiveState() ? natural_scale : max_scale;
  while (!ragged && page_width(scale_limit) < available && scale_limit < natural_scale * 64) {
    scale_limit *= 2;
  }

  float scale = scale_limit;
  if (page_width(scale_limit) > available) {
    float lo = 0;
    float hi = scale_limit;
    for (int i = 0; i < 24; ++i) {
      const float mid = (lo + hi) / 2;
      if (page_width(mid) > available) {
        hi = mid;
      }
      else {
        lo = mid;
      }
    }
    scale = lo;
  }

  open_ties.clear();
  drawn_ties.clear();
  head_boxes.clear();
  label_obstacles.clear();
  measure_labels.clear();
  add_obstacle(clef_x, clef_x + clef_width, findStaveY(4, STAVE_TREBLE) - clef_height, findStaveY(10, STAVE_TREBLE));

  int tick_delta = std::numeric_limits<int>::min();
  int tick_offset = offset;
  int barline_x = offset;
  for (int m = measureRange.first; m <= measureRange.second; ++m) {
    const sheetMeasure& dm = displayMeasure[m - 1];

    // beginning of measure spacing (not first measure)
    if (m != measureRange.first) {
      offset += sigSpacing;
    }

    int tick_diff = dm.measure->getLocation() - ctr.timeOffset;
    if (tick_diff > tick_delta && tick_diff < 0) {
      tick_delta = tick_diff;
      tick_offset = offset;
    }

    if (m == measureRange.first && dm.measure->keySignatures.empty()) {
      drawKeySignature(dm.measure->currentKey, offset);
      offset += getKeyWidth(dm.measure->currentKey);
    }
    else {
      for (const auto& ks : dm.measure->keySignatures) {
        if (dm.measure->currentKey.getAcc() != 0) {
          drawKeySignature(ks, offset);
          offset += getKeyWidth(ks) + sigSpacing;
        }
      }
      for (const auto& ts : dm.measure->timeSignatures) {
        drawTimeSignature(ts, offset);
        offset += getTimeWidth(ts) + sigSpacing;
      }
    }

    const int measure_x = offset;
    measure_labels.push_back({m, measure_x + sigSpacing / 20.0f, static_cast<float>(barline_x),
                              m == measureRange.first});

    // lay out chord positions first, beams need every stem position up front
    float cursor = offset + lead_pad;
    if (dm.chords.empty()) {
      const float rest_w = getSymbolWidth(SYM_REST_WHOLE);
      const float space = std::max(dm.rest_ideal * scale, rest_w + lead_pad);
      draw_rest(NOTE_WHOLE, STAVE_TREBLE, cursor + (space - head_width_exact) / 2);
      draw_rest(NOTE_WHOLE, STAVE_BASS, cursor + (space - head_width_exact) / 2);
      cursor += space;
    }
    else {
      cursor += dm.s_chordData.front().leftWidth;
    }

    chord_x.resize(dm.chords.size());
    for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
      chord_x[ch] = std::lround(cursor);
      for (const sheetNote* n : dm.chords[ch].second) {
        if (n->tie_in) {
          continue;
        }
        int note_diff = stream.notes[n->oriNote].x - ctr.timeOffset;
        if (note_diff > tick_delta && note_diff < 0) {
          tick_delta = note_diff;
          tick_offset = chord_x[ch] - dm.s_chordData[ch].leftWidth;
        }
      }
      cursor += std::max(dm.s_chordData[ch].ideal * scale, static_cast<float>(dm.s_chordData[ch].min_space));
    }
    offset = std::lround(cursor);

    draw_measure(dm, measure_x, offset);

    // end of measure spacing
    offset += sigSpacing;

    // end of measure bars (except the end)
    if (m != measureRange.second) {
      drawLineEx(offset, ctr.menuHeight + ctr.barMargin, offset,
                 ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
      barline_x = offset;
      // double bar line on time/key changes
      if (!displayMeasure[m].measure->keySignatures.empty() ||
          !displayMeasure[m].measure->timeSignatures.empty()) {
        offset += sigSpacing;
        drawLineEx(offset, ctr.menuHeight + ctr.barMargin, offset,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
        barline_x = offset;
      }
    }

    // thicker double bar on final measure
    if (displayMeasure.back().measure->getNumber() == m) {
      if (!ctr.getLiveState()) {
        offset += endSpacing + sigSpacing + 1;
        drawLineEx(offset, ctr.menuHeight + ctr.barMargin, offset,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
        offset += sigSpacing + 2;
        drawLineEx(offset, ctr.menuHeight + ctr.barMargin, offset,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing + 1, endSpacing,
                   ctr.bgSheetNote);
        // a short final page ends its staves at the closing barline
        staff_end = std::min<float>(staff_end, offset);
      }
    }
  }

  // ties continuing onto the next page run to the end of the system
  const float system_end = std::min<float>(offset, ctr.getWidth() - ctr.sheetSideMargin) - 2;
  for (const auto& t : open_ties) {
    const float x1 = std::max(t.x + tie_min_len, system_end);
    draw_tie(t.x, x1, t.y, t.up, t.flip_y, t.head_note, t.note);
  }

  place_measure_labels();

  // arrow at current measure
  Vector2 m_text_pos = {(float)tick_offset + sigSpacing / 20.f,
                        ctr.menuHeight + ctr.barMargin - ctr.barWidth * 1.75};

  constexpr float arrow_size = 10;
  constexpr float arrow_size2 = 4;
  drawLineEx(m_text_pos.x, ctr.menuHeight, m_text_pos.x, ctr.menuHeight + arrow_size + 1, 1, ctr.bgNow);
  drawLineEx(m_text_pos.x - arrow_size2, ctr.menuHeight + arrow_size - arrow_size2, m_text_pos.x,
             ctr.menuHeight + arrow_size, 1, ctr.bgNow);
  drawLineEx(m_text_pos.x + arrow_size2 + 1, ctr.menuHeight + arrow_size - arrow_size2, m_text_pos.x + 1,
             ctr.menuHeight + arrow_size, 1, ctr.bgNow);
}

void sheetController::add_obstacle(float x0, float x1, float top, float bottom) {
  // only what rises above the treble stave can meet a measure number
  if (top < ctr.menuHeight + ctr.barMargin) {
    label_obstacles.push_back({x0, x1, top, bottom});
  }
}

float sheetController::find_label_y(float x, float width, float ink_top, float ink_bottom, float y) const {
  // at its default height a label may sit under floating ties, but once displaced it goes over the whole
  // column, so it never ends up wedged between ties and notes
  float column_top = std::numeric_limits<float>::max();
  bool clear = true;
  for (const auto& ob : label_obstacles) {
    if (ob.x1 >= x - label_pad && ob.x0 <= x + width + label_pad) {
      column_top = std::min(column_top, ob.top);
      clear &= ob.bottom <= y + ink_top - label_pad;
    }
  }
  return clear ? y : std::min(y, column_top - ink_bottom - label_pad);
}

void sheetController::place_measure_labels() {
  const float default_y = ctr.menuHeight + ctr.barMargin - ctr.barWidth * 1.75f;

  // digits sit well inside the text box, so collide with their ink rather than the box
  const Font& font = ctr.getFont(DEFAULT_FONT, label_size);
  float ink_top = std::numeric_limits<float>::max();
  float ink_bottom = 0;
  for (int digit = '0'; digit <= '9'; ++digit) {
    const GlyphInfo glyph = GetGlyphInfo(font, digit);
    ink_top = std::min(ink_top, static_cast<float>(glyph.offsetY));
    ink_bottom = std::max(ink_bottom, static_cast<float>(glyph.offsetY + glyph.image.height));
  }

  for (const auto& label : measure_labels) {
    const string text = to_string(label.number);
    const float width = measureTextEx(text, label_size).x;

    // slide outwards a pixel at a time, across the barline or back over the signatures,
    // and keep the spot that stays closest to the default height
    const float lo = label.first ? label.x - width - label_slide_back : label.barline - width - label_pad;
    const float hi = label.x + label_slide;
    float best_x = label.x;
    float best_y = find_label_y(label.x, width, ink_top, ink_bottom, default_y);
    for (int step = 1; best_y < default_y; ++step) {
      bool in_range = false;
      for (const float x : {label.x - step, label.x + step}) {
        if (x < lo || x > hi) {
          continue;
        }
        in_range = true;
        const float y = find_label_y(x, width, ink_top, ink_bottom, default_y);
        if (y > best_y + 0.5f) {
          best_x = x;
          best_y = y;
        }
      }
      if (!in_range) {
        break;
      }
    }

    drawTextEx(text, {best_x, std::max(best_y, static_cast<float>(ctr.menuHeight))}, ctr.bgSheetNote, 255,
               label_size);
  }
}

void sheetController::draw_measure(const sheetMeasure& dm, int measure_x, int measure_end) {
  beam_lines.resize(dm.beams.size());
  for (unsigned int b = 0; b < dm.beams.size(); ++b) {
    beam_lines[b] = find_beam_line(dm, dm.beams[b]);
  }

  // how far each chord reaches above and below the staves, so markers can stack past it
  chord_top.assign(dm.chords.size(), findStaveY(10, STAVE_TREBLE));
  chord_bottom.assign(dm.chords.size(), findStaveY(-10, STAVE_BASS));

  // shapes and glyphs use different textures, so each gets one pass to keep raylib batching them
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    if constexpr (debug_layout) {
      drawLineEx(chord_x[ch], ctr.menuHeight + ctr.barMargin, chord_x[ch],
                 ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 1, ctr.bgNow);
    }
    draw_group_shapes(dm, ch, STAVE_TREBLE, measure_x);
    draw_group_shapes(dm, ch, STAVE_BASS, measure_x);
    draw_oob_markers(dm, ch);
  }
  for (unsigned int b = 0; b < dm.beams.size(); ++b) {
    draw_beam(dm, b);
  }

  const int head = getSymbolWidth(SYM_HEAD_STD);
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      const int rest = dm.s_chordData[ch].rests[stave];
      if (rest >= 0) {
        draw_rest(rest, stave, chord_x[ch] + (head - getSymbolWidth(get_rest_symbol(rest))) / 2);
      }
    }
    draw_group_glyphs(dm, ch, STAVE_TREBLE);
    draw_group_glyphs(dm, ch, STAVE_BASS);
  }

  // a stave silent for the whole measure gets one centered whole rest
  if (!dm.chords.empty()) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      if (dm.bar_rest[stave]) {
        draw_rest(NOTE_WHOLE, stave, (measure_x + measure_end - head_width_exact) / 2);
      }
    }
  }
}

int sheetController::find_rest_y(int type, int stave) {
  // whole rests hang from the second line, the rest sit on the middle line
  const int line = type == NOTE_WHOLE ? (stave == STAVE_TREBLE ? 8 : -4) : (stave == STAVE_TREBLE ? 6 : -6);
  return findStaveY(line, stave) + (type == NOTE_WHOLE ? 1 : 0);
}

float sheetController::find_rest_extent(int type, bool below) {
  // how far each rest glyph reaches from where it is placed, measured from leland
  switch (type) {
    case NOTE_WHOLE:
      return below ? 6 : 0;
    case NOTE_HALF:
      return below ? 1 : 6;
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
      return below ? 13 : 16;
    case NOTE_8:
      return below ? 10 : 8;
    case NOTE_16:
      return below ? 20 : 8;
    case NOTE_32:
      return below ? 20 : 18;
    default:
      return below ? 30 : 18;
  }
}

void sheetController::draw_rest(int type, int stave, float x) {
  const int y = find_rest_y(type, stave);
  const int sym = get_rest_symbol(type);
  drawSymbol(sym, fSize, x, y - glyph_baseline, ctr.bgSheetNote);

  if (is_dotted_type(type)) {
    drawSymbol(SYM_DOT, fSize, x + getSymbolWidth(sym) + dotSpacing, y - ctr.barWidth / 2 - glyph_baseline,
               ctr.bgSheetNote);
  }
}

void sheetController::draw_oob_markers(const sheetMeasure& dm, int ch) {
  const sheetChordData& cd = dm.s_chordData[ch];
  const float x = chord_x[ch] + head_width_exact / 2;
  const float half = marker_width / 2;

  // pinned to the top and bottom edges of the sheet, out of the way of the notes
  if (cd.oob_high) {
    const float tip = ctr.menuHeight + marker_gap;
    const Vector2 left[2] = {{x - half, tip + marker_height}, {x, tip}};
    const Vector2 right[2] = {{x + half, tip + marker_height}, {x, tip}};
    draw_aa_strip(left, right, 2, ctr.bgSheetNote);
    add_obstacle(x - half, x + half, tip, tip + marker_height);
  }
  if (cd.oob_low) {
    const float tip = ctr.menuHeight + ctr.barHeight - marker_gap;
    const Vector2 left[2] = {{x - half, tip - marker_height}, {x, tip}};
    const Vector2 right[2] = {{x + half, tip - marker_height}, {x, tip}};
    draw_aa_strip(left, right, 2, ctr.bgSheetNote);
  }
}

float sheetController::find_stem_x(int anchor, int dir) const {
  // stems sit on the right edge of the notehead going up, the left edge going down, on whole pixels to stay crisp
  return std::round(dir == FLAG_DOWN ? anchor + stemWidth / 2.0f : anchor + head_width_exact - stemWidth / 2.0f);
}

int sheetController::find_head_x(const stem_group& g, const sheetNote* n, int anchor) {
  // a flipped notehead sits on the far side of the stem
  if (g.dir == FLAG_DOWN && n->left) {
    return anchor - getSymbolWidth(SYM_HEAD_STD);
  }
  if (g.dir != FLAG_DOWN && !n->left) {
    return anchor + getSymbolWidth(SYM_HEAD_STD);
  }
  return anchor;
}

int sheetController::find_dot_x(const sheetMeasure& dm, int ch, const stem_group& g, const sheetNote* n,
                                int stave) {
  // a dot follows whatever is on its own row, so it only moves past a flipped notehead right beside it
  const midi& stream = ctr.getStream();
  const auto& c = dm.chords[ch].second;
  const int sheet_y = stream.notes[n->oriNote].sheetY;
  const int row = sheet_y % 2 == 0 ? sheet_y + 1 : sheet_y;

  int right = g.head_width;
  if (g.flipped && g.dir != FLAG_DOWN) {
    for (int i = g.first; i <= g.last; ++i) {
      const sheetNote* m = c[i];
      if (m->visible && !m->out_of_bounds && m->stave == stave && !m->left &&
          std::abs(stream.notes[m->oriNote].sheetY - row) <= 1) {
        right = getSymbolWidth(SYM_HEAD_STD) + g.head_width;
      }
    }
  }
  return right + dotSpacing;
}

int sheetController::find_flag_y(const stem_group& g, int stave) {
  // stems stay close to their notes, longer only when extra flags need the room
  const int len = stem_len + (g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0);
  // even when nothing else works, a stem stops at the edge of the sheet
  return g.dir == FLAG_DOWN ? std::min<int>(findStaveY(g.stem_lo, stave) + len, find_sheet_bottom())
                            : std::max<int>(findStaveY(g.stem_hi, stave) - len, find_sheet_top());
}

void sheetController::draw_group_shapes(const sheetMeasure& dm, int ch, int stave, int measure_x) {
  const stem_group& g = dm.s_chordData[ch].groups[stave];
  if (g.empty()) {
    return;
  }

  const midi& stream = ctr.getStream();
  const auto& c = dm.chords[ch].second;
  const int anchor = chord_x[ch];
  const bool down = g.dir == FLAG_DOWN;

  float top = findStaveY(g.hi, stave) - head_half;
  float bottom = findStaveY(g.lo, stave) + head_half;

  draw_ledger_lines(dm, ch, g, stave, anchor);

  for (int i = g.first; i <= g.last; ++i) {
    const sheetNote* n = c[i];
    if (!n->visible || n->out_of_bounds || n->stave != stave) {
      continue;
    }

    const int y = findStaveY(stream.notes[n->oriNote].sheetY, stave);
    const int head_x = find_head_x(g, n, anchor);
    const int head_sym = getSymbolType(n->display_type);
    head_boxes.push_back({static_cast<float>(head_x), static_cast<float>(head_x + getSymbolWidth(head_sym)),
                          y - head_half, y + head_half, n->oriNote});
    if (stave == STAVE_TREBLE) {
      add_obstacle(head_x, head_x + getSymbolWidth(head_sym), y - head_half, y + head_half);
    }

    if (has_acc(n)) {
      top = std::min<float>(top, y - acc_above);
      bottom = std::max<float>(bottom, y + acc_below);
      if (stave == STAVE_TREBLE) {
        const int acc_sym = getAccType(n->displayAcc);
        const int acc_w = getSymbolWidth(acc_sym);
        const float acc_x = anchor - g.acc_x - n->acc_col * acc_col_width - acc_w;
        add_obstacle(acc_x, acc_x + acc_w, y - find_acc_height(acc_sym), y + acc_below);
      }
    }
    if (!n->tie_in && !n->tie_out) {
      continue;
    }

    const int head_w = getSymbolWidth(head_sym);
    const int tie_off = n->tie_outside ? tie_outside_offset : tie_offset;
    const float tie_y = y + (n->tie_up ? -tie_off : tie_off);
    const float flip_y = y + (n->tie_up ? tie_off : -tie_off);
    top = std::min(top, tie_y - tie_max_height);
    bottom = std::max(bottom, tie_y + tie_max_height);

    // ties around the heads run from centre to centre, ties between them from edge to edge
    auto find_tie_end = [&](bool outside) { return outside ? head_x + head_w * 0.35f : head_x - 2.0f; };

    if (n->tie_in) {
      auto it = std::ranges::find(open_ties, n->oriNote, &open_tie::note);
      if (it != open_ties.end()) {
        const float tie_end = find_tie_end(it->outside);
        draw_tie(it->x, tie_end, it->y, it->up, it->flip_y, it->head_note, n->oriNote);
        open_ties.erase(it);
      }
      else {
        // the tie started on an earlier page, so it picks up from the start of the measure
        const float tie_end = find_tie_end(n->tie_outside);
        const float tie_x = std::min<float>(measure_x + 2, tie_end - tie_min_len);
        draw_tie(tie_x, tie_end, tie_y, n->tie_up, flip_y, n->oriNote, n->oriNote);
      }
    }
    if (n->tie_out) {
      // a dotted note's tie starts after its dot
      float tie_x = n->tie_outside ? head_x + head_w * 0.65f : head_x + head_w + 2.0f;
      if (is_dotted_type(n->display_type)) {
        tie_x = anchor + find_dot_x(dm, ch, g, n, stave) + getSymbolWidth(SYM_DOT) + 2;
      }
      open_ties.push_back({n->oriNote, tie_x, tie_y, n->tie_up, n->tie_outside, flip_y, n->oriNote});
    }
  }

  // a duplicate hidden behind a note at the same position still carries its tie, so it lands on that note
  for (const sheetNote* hidden : c) {
    if (hidden->visible || hidden->out_of_bounds || hidden->stave != stave || !(hidden->tie_in || hidden->tie_out)) {
      continue;
    }
    const int sheet_y = stream.notes[hidden->oriNote].sheetY;
    auto twin = std::ranges::find_if(c, [&](const sheetNote* n) {
      return n->visible && !n->out_of_bounds && n->stave == stave && stream.notes[n->oriNote].sheetY == sheet_y;
    });
    if (twin == c.end()) {
      continue;
    }

    const int head_x = find_head_x(g, *twin, anchor);
    const int head_w = getSymbolWidth(getSymbolType((*twin)->display_type));
    if (hidden->tie_in) {
      auto it = std::ranges::find(open_ties, hidden->oriNote, &open_tie::note);
      if (it != open_ties.end()) {
        draw_tie(it->x, it->outside ? head_x + head_w * 0.35f : head_x - 2.0f, it->y, it->up, it->flip_y,
                 it->head_note, (*twin)->oriNote);
        open_ties.erase(it);
      }
    }
    if (hidden->tie_out) {
      const int y = findStaveY(sheet_y, stave);
      const int tie_off = (*twin)->tie_outside ? tie_outside_offset : tie_offset;
      const float tie_y = y + ((*twin)->tie_up ? -tie_off : tie_off);
      const float flip_y = y + ((*twin)->tie_up ? tie_off : -tie_off);
      float tie_x = (*twin)->tie_outside ? head_x + head_w * 0.65f : head_x + head_w + 2.0f;
      if (is_dotted_type((*twin)->display_type)) {
        tie_x = anchor + find_dot_x(dm, ch, g, *twin, stave) + getSymbolWidth(SYM_DOT) + 2;
      }
      open_ties.push_back(
          {hidden->oriNote, tie_x, tie_y, (*twin)->tie_up, (*twin)->tie_outside, flip_y, (*twin)->oriNote});
    }
  }

  if (g.has_stem()) {
    const float stem_x = find_stem_x(anchor, g.dir);
    const float base = down ? findStaveY(g.stem_hi, stave) + stem_attach : findStaveY(g.stem_lo, stave) - stem_attach;

    float tip = 0;
    if (g.beam >= 0) {
      const beam_line& line = beam_lines[g.beam];
      tip = line.y0 + line.slope * (stem_x - line.x0);
    }
    else {
      tip = find_flag_y(g, stave);
    }

    drawLineEx(stem_x, base, stem_x, tip, stemWidth, ctr.bgSheetNote);
    if (g.has_flag()) {
      draw_flags(stem_x - stemWidth / 2.0f, tip, g.flag_type - FLAGTYPE_8 + 1, down);
    }
    top = std::min(top, tip);
    bottom = std::max(bottom, tip);

    if (stave == STAVE_TREBLE) {
      const float flag_w = g.has_flag() ? getSymbolWidth(g.flag_type, g.dir) : 0;
      add_obstacle(stem_x - stemWidth / 2.0f, stem_x + stemWidth / 2.0f + flag_w, std::min(base, tip),
                   std::max(base, tip));
    }
  }

  if (stave == STAVE_TREBLE) {
    chord_top[ch] = std::min(chord_top[ch], top);
  }
  else {
    chord_bottom[ch] = std::max(chord_bottom[ch], bottom);
  }
}

void sheetController::draw_group_glyphs(const sheetMeasure& dm, int ch, int stave) {
  const stem_group& g = dm.s_chordData[ch].groups[stave];
  if (g.empty()) {
    return;
  }

  const midi& stream = ctr.getStream();
  const colorRGB& col = ctr.bgSheetNote;
  const auto& c = dm.chords[ch].second;
  const int anchor = chord_x[ch];

  for (int i = g.first; i <= g.last; ++i) {
    const sheetNote* n = c[i];
    if (!n->visible || n->out_of_bounds || n->stave != stave) {
      continue;
    }

    const note& src = stream.notes[n->oriNote];
    const int y = findStaveY(src.sheetY, stave);
    drawSymbol(getSymbolType(n->display_type), fSize, find_head_x(g, n, anchor), y - glyph_baseline, col);

    if (has_acc(n)) {
      const int acc_sym = getAccType(n->displayAcc);
      const int acc_x = anchor - g.acc_x - n->acc_col * acc_col_width - getSymbolWidth(acc_sym);
      drawSymbol(acc_sym, fSize, acc_x, y - glyph_baseline, col);
    }

    if (is_dotted_type(n->display_type)) {
      // dots on a line move up into the space
      const int dot_y = src.sheetY % 2 == 0 ? y - ctr.barWidth / 2 : y;
      drawSymbol(SYM_DOT, fSize, anchor + find_dot_x(dm, ch, g, n, stave), dot_y - glyph_baseline, col);
    }
  }

}

void sheetController::draw_flags(float x, float tip, int count, bool down) {
  // the font's flags have hairline tails that break up at this size, so a thickened trace is drawn instead
  const float scale_y = count == 1 ? flag_scale_single : flag_scale_stacked;
  const float dir = down ? -1.0f : 1.0f;

  for (int k = 0; k < count; ++k) {
    const float y0 = tip + dir * k * flag_step;
    Vector2 outer[flag_trace_len];
    Vector2 inner[flag_trace_len];
    for (int i = 0; i < flag_trace_len; ++i) {
      outer[i] = {x + flag_outer[i][0], y0 + dir * flag_outer[i][1] * scale_y};
      inner[i] = {x + flag_inner[i][0], y0 + dir * flag_inner[i][1] * scale_y};
    }
    draw_aa_strip(outer, inner, flag_trace_len, ctr.bgSheetNote);
  }
}

void sheetController::draw_ledger_lines(const sheetMeasure& dm, int ch, const stem_group& g, int stave,
                                        int anchor) {
  const midi& stream = ctr.getStream();
  const auto& c = dm.chords[ch].second;
  const int top_line = stave == STAVE_TREBLE ? 10 : -2;
  const int bottom_line = stave == STAVE_TREBLE ? 2 : -10;

  // each ledger line only reaches across the noteheads at or beyond it
  auto draw_ledger = [&](int line, bool above) {
    float x0 = std::numeric_limits<float>::max();
    float x1 = std::numeric_limits<float>::lowest();
    for (int i = g.first; i <= g.last; ++i) {
      const sheetNote* n = c[i];
      if (!n->visible || n->out_of_bounds || n->stave != stave) {
        continue;
      }
      const int sheet_y = stream.notes[n->oriNote].sheetY;
      if (above ? sheet_y < line : sheet_y > line) {
        continue;
      }
      const int head_x = find_head_x(g, n, anchor);
      x0 = std::min<float>(x0, head_x - ledger_ext);
      x1 = std::max<float>(x1, head_x + getSymbolWidth(getSymbolType(n->display_type)) + ledger_ext);
    }
    if (x0 >= x1) {
      return;
    }
    const int y = findStaveY(line, stave);
    drawLineEx(x0, y, x1, y, 1, ctr.bgSheetNote);
    if (above && stave == STAVE_TREBLE) {
      add_obstacle(x0, x1, y - 1, y + 1);
    }
  };

  for (int line = top_line + 2; line <= g.hi; line += 2) {
    draw_ledger(line, true);
  }
  for (int line = bottom_line - 2; line >= g.lo; line -= 2) {
    draw_ledger(line, false);
  }
}

sheetController::beam_line sheetController::find_beam_line(const sheetMeasure& dm, const beam_group& beam) {
  const bool up = beam.dir == FLAG_UP;

  // the notehead closest to the beam
  auto near_y = [&](int ch) {
    const stem_group& g = dm.s_chordData[ch].groups[beam.stave];
    return findStaveY(up ? g.stem_hi : g.stem_lo, beam.stave);
  };

  const int first = beam.chords.front();
  const int last = beam.chords.back();
  const float x0 = find_stem_x(chord_x[first], beam.dir);
  const float dx = find_stem_x(chord_x[last], beam.dir) - x0;

  float slope = 0;
  if (dx > 0) {
    const int y_first = near_y(first);
    const int y_last = near_y(last);
    const float max_rise = std::min(beam_max_rise, dx * beam_max_slope);
    slope = std::clamp(static_cast<float>(y_last - y_first), -max_rise, max_rise) / dx;

    // stay flat when an inner note reaches past both ends
    for (unsigned int i = 1; i + 1 < beam.chords.size(); ++i) {
      const int y = near_y(beam.chords[i]);
      if (up ? y < std::min(y_first, y_last) : y > std::max(y_first, y_last)) {
        slope = 0;
        break;
      }
    }
  }

  int levels = 1;
  for (int ch : beam.chords) {
    levels = std::max(levels, dm.s_chordData[ch].groups[beam.stave].flag_type - FLAGTYPE_8 + 1);
  }
  const float len = beam_stem_len + (levels - 1) * beam_step;

  // shift the beam until every stem is long enough
  float y0 = up ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
  for (int ch : beam.chords) {
    const float x = find_stem_x(chord_x[ch], beam.dir) - x0;
    float tip = up ? near_y(ch) - len : near_y(ch) + len;

    // the beam also clears a stemless note sitting on its side of the chord
    const stem_group& g = dm.s_chordData[ch].groups[beam.stave];
    const float clear = head_half + 2 + (levels - 1) * beam_step + beam_thickness;
    tip = up ? std::min(tip, findStaveY(g.hi, beam.stave) - clear) : std::max(tip, findStaveY(g.lo, beam.stave) + clear);

    y0 = up ? std::min(y0, tip - slope * x) : std::max(y0, tip - slope * x);
  }

  // pull back towards the notes to clear the other stave, but never past the shortest allowed stem
  if (beam.limited) {
    const float min_len = len - (beam_stem_len - beam_min_len);
    float shift = 0;
    float room = std::numeric_limits<float>::max();
    for (int ch : beam.chords) {
      const float line_y = y0 + slope * (find_stem_x(chord_x[ch], beam.dir) - x0);
      shift = std::max(shift, up ? beam.limit - line_y : line_y - beam.limit);
      room = std::min(room, up ? near_y(ch) - min_len - line_y : line_y - near_y(ch) - min_len);

      // nor past a stemless note on the beam's side, which matters more than a little room from the other stave
      const stem_group& g = dm.s_chordData[ch].groups[beam.stave];
      const float clear = head_half + 2 + (levels - 1) * beam_step + beam_thickness;
      room = std::min(room, up ? findStaveY(g.hi, beam.stave) - clear - line_y
                               : line_y - findStaveY(g.lo, beam.stave) - clear);
    }
    y0 += (up ? 1.0f : -1.0f) * std::clamp(shift, 0.0f, std::max(room, 0.0f));
  }

  // even when nothing else works, a beam stays inside the sheet (its other levels stack towards the notes)
  for (int ch : beam.chords) {
    const float line_y = y0 + slope * (find_stem_x(chord_x[ch], beam.dir) - x0);
    if (up && line_y < find_sheet_top()) {
      y0 += find_sheet_top() - line_y;
    }
    else if (!up && line_y > find_sheet_bottom()) {
      y0 -= line_y - find_sheet_bottom();
    }
  }

  return {x0, y0, slope};
}

void sheetController::draw_beam(const sheetMeasure& dm, int b) {
  const beam_group& beam = dm.beams[b];
  const beam_line& line = beam_lines[b];
  const bool up = beam.dir == FLAG_UP;
  const int count = beam.chords.size();
  const float half = stemWidth / 2.0f;

  auto stem_x = [&](int i) { return find_stem_x(chord_x[beam.chords[i]], beam.dir); };
  auto levels = [&](int i) { return dm.s_chordData[beam.chords[i]].groups[beam.stave].flag_type - FLAGTYPE_8 + 1; };

  // secondary beams stack from the outer edge towards the noteheads
  auto segment = [&](float xa, float xb, int level) {
    const float shift = (up ? 1.0f : -1.0f) * level * beam_step;
    const float ya = line.y0 + line.slope * (xa - line.x0) + shift;
    const float yb = line.y0 + line.slope * (xb - line.x0) + shift;
    const float top = up ? 0 : -beam_thickness;
    const float bottom = up ? beam_thickness : 0;
    const Vector2 upper[2] = {{xa, ya + top}, {xb, yb + top}};
    const Vector2 lower[2] = {{xa, ya + bottom}, {xb, yb + bottom}};
    draw_aa_strip(upper, lower, 2, ctr.bgSheetNote);
  };

  segment(stem_x(0) - half, stem_x(count - 1) + half, 0);

  int max_level = 0;
  for (int i = 0; i < count; ++i) {
    max_level = std::max(max_level, levels(i));
  }

  if (beam.stave == STAVE_TREBLE) {
    const float xa = stem_x(0) - half;
    const float xb = stem_x(count - 1) + half;
    const float ya = line.y0 + line.slope * (xa - line.x0);
    const float yb = line.y0 + line.slope * (xb - line.x0);
    const float depth = (max_level - 1) * beam_step + beam_thickness;
    add_obstacle(xa, xb, std::min(ya, yb) - (up ? 0 : depth), std::max(ya, yb) + (up ? depth : 0));
  }

  for (int level = 1; level < max_level; ++level) {
    for (int i = 0; i < count;) {
      if (levels(i) <= level) {
        ++i;
        continue;
      }
      int j = i;
      while (j + 1 < count && levels(j + 1) > level) {
        ++j;
      }

      if (j > i) {
        segment(stem_x(i) - half, stem_x(j) + half, level);
      }
      else if (i == 0) {
        // a lone shorter note gets a partial beam pointing into the group
        segment(stem_x(0) - half, stem_x(0) + std::min(beamlet_width, (stem_x(1) - stem_x(0)) / 2), level);
      }
      else {
        segment(stem_x(i) - std::min(beamlet_width, (stem_x(i) - stem_x(i - 1)) / 2), stem_x(i) + half, level);
      }
      i = j + 1;
    }
  }
}

float sheetController::find_tie_arc(float t, float shoulder) const {
  // flat at the ends' height, rising with shoulders that grow on longer ties
  return 1.0f - std::pow(std::abs(2.0f * t - 1.0f), shoulder);
}

bool sheetController::tie_hits_heads(float x0, float x1, float y, bool up, float height, float shoulder,
                                     float thickness, int start_note, int end_note) const {
  const float len = x1 - x0;
  const float sign = up ? -1.0f : 1.0f;
  for (const auto& head : head_boxes) {
    if (head.note == start_note || head.note == end_note || head.x1 <= x0 || head.x0 >= x1) {
      continue;
    }
    // the lens over the head's width, sampled at its edges and where it comes closest to the middle
    const float t0 = std::clamp((head.x0 - x0) / len, 0.0f, 1.0f);
    const float t1 = std::clamp((head.x1 - x0) / len, 0.0f, 1.0f);
    float reach_lo = std::numeric_limits<float>::max();
    float reach_hi = std::numeric_limits<float>::lowest();
    for (const float t : {t0, std::clamp(0.5f, t0, t1), t1}) {
      const float arc = find_tie_arc(t, shoulder);
      const float outer = y + sign * (height * arc + tie_end_width / 2);
      const float inner = y + sign * ((height - thickness) * arc - tie_end_width / 2);
      reach_lo = std::min({reach_lo, outer, inner});
      reach_hi = std::max({reach_hi, outer, inner});
    }
    if (reach_hi > head.y0 - tie_head_clearance && reach_lo < head.y1 + tie_head_clearance) {
      return true;
    }
  }
  return false;
}

void sheetController::draw_tie(float x0, float x1, float y, bool up, float flip_y, int start_note, int end_note) {
  if (x1 - x0 < tie_min_len) {
    const float mid = (x0 + x1) / 2;
    x0 = mid - tie_min_len / 2;
    x1 = mid + tie_min_len / 2;
  }

  // doubled notes tied to doubled notes would stack identical ties, so only the first is drawn
  for (const auto& t : drawn_ties) {
    if (t.up == up && std::abs(t.y - y) < 3 && std::abs(t.x1 - x1) < 3 && std::abs(t.x0 - x0) < tie_merge_gap) {
      return;
    }
  }
  drawn_ties.push_back({x0, x1, y, up});

  constexpr int segments = 16;
  const float len = x1 - x0;

  // longer ties rise higher but more slowly, and grow shoulders so the middle runs flatter
  const float height = std::clamp(std::sqrt(len) * 1.1f, tie_min_height, tie_max_height);
  const float shoulder = std::clamp(2.0f + len / 120.0f, 2.0f, 3.5f);
  const float thickness = std::clamp(1.8f + len * 0.004f, 1.8f, 2.6f);

  // a tie may cross stems but tries not to cross noteheads, first by reshaping and then by changing sides
  struct candidate {
    float y;
    bool up;
    float height;
  };
  const float flatter = std::max(tie_min_height, height * 0.5f);
  const candidate candidates[] = {
      {y, up, height},      {y, up, flatter},           {y, up, height * 1.6f},     {flip_y, !up, height},
      {flip_y, !up, flatter}, {y, up, height * 2.2f},   {flip_y, !up, height * 1.6f},
  };
  candidate chosen = candidates[0];
  for (const auto& c : candidates) {
    if (!tie_hits_heads(x0, x1, c.y, c.up, c.height, shoulder, std::min(thickness, c.height * 0.7f), start_note,
                        end_note)) {
      chosen = c;
      break;
    }
  }

  const float sign = chosen.up ? -1.0f : 1.0f;
  const float lens = std::min(thickness, chosen.height * 0.7f);
  add_obstacle(x0, x1, chosen.up ? chosen.y - chosen.height - 1 : chosen.y,
               chosen.up ? chosen.y : chosen.y + chosen.height + 1);

  // a lens between two arcs, thick in the middle and thin at the ends
  Vector2 outer[segments + 1];
  Vector2 inner[segments + 1];
  for (int i = 0; i <= segments; ++i) {
    const float t = static_cast<float>(i) / segments;
    const float arc = find_tie_arc(t, shoulder);
    const float x = x0 + len * t;
    outer[i] = {x, chosen.y + sign * (chosen.height * arc + tie_end_width / 2)};
    inner[i] = {x, chosen.y + sign * ((chosen.height - lens) * arc - tie_end_width / 2)};
  }
  draw_aa_strip(outer, inner, segments + 1, ctr.bgSheetNote);
}

int sheetController::getDisplayAccType(int& DFAState, int noteAccType) {
  switch (DFAState) {
    case DA_STATE_CLEAR:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_CLEAR;
          return ACC_NONE;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_SHARP:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP_MULT;
          return ACC_NONE;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_FLAT:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT_MULT;
          return ACC_NONE;
      }
      break;
    case DA_STATE_NATURAL:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_CLEAR;
          return ACC_NONE;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_SHARP_MULT:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP_MULT;
          return ACC_NONE;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_FLAT_MULT:
      switch (noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT_MULT;
          return ACC_NONE;
      }
      break;
    default:
      logW(LL_WARN, "invalid DFA state or accidental type for",
           "display accidental calculation (state, type):", DFAState, noteAccType);
      break;
  }

  return -1;
}

int sheetController::getSymbolWidth(const int symbol) {
  switch (symbol) {
    case SYM_HEAD_WHOLE:
      return 15;
    case SYM_HEAD_HALF:
      return 13;
    case SYM_HEAD_STD:
      return 13;
    case SYM_FLAG_8U:
      return 12;
    case SYM_FLAG_8D:
      return 13;
    case SYM_FLAG_16U:
      return 11;
    case SYM_FLAG_16D:
      return 13;
    case SYM_FLAG_32U:
      return 11;
    case SYM_FLAG_32D:
      return 13;
    case SYM_FLAG_64U:
      return 11;
    case SYM_FLAG_64D:
      return 13;
    case SYM_ACC_FLAT:
      return 8;
    case SYM_ACC_NATURAL:
      return 7;
    case SYM_ACC_SHARP:
      return 10;
    case SYM_DOT:
      return 4;
    case SYM_REST_WHOLE:
      return 18;
    case SYM_REST_HALF:
      return 13;
    case SYM_REST_4:
      return 9;
    case SYM_REST_8:
      return 11;
    case SYM_REST_16:
      return 13;
    case SYM_REST_32:
      return 15;
    case SYM_REST_64:
      return 17;
    default:
      logQ("invalid symbol:", symbol);
      break;
  }

  return -1;
}

int sheetController::getSymbolWidth(const int flagType, const int dir) {
  switch (flagType) {
    case FLAGTYPE_STEM:
      return stemWidth;
    case FLAGTYPE_8:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_8U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_8D);
      }
      break;
    case FLAGTYPE_16:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_16U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_16D);
      }
      break;
    case FLAGTYPE_32:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_32U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_32D);
      }
      break;
    case FLAGTYPE_64:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_64U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_64D);
      }
      break;
    case FLAGTYPE_NONE:
      return 0;
  }

  return -1;
}

int sheetController::getAccType(const int accType) {
  switch (accType) {
    case ACC_FLAT:
      return SYM_ACC_FLAT;
    case ACC_SHARP:
      return SYM_ACC_SHARP;
    case ACC_NATURAL:
      return SYM_ACC_NATURAL;
    default:
      return 0;
  }
}

int sheetController::find_acc_height(const int symbol) {
  // flats reach furthest above their note
  return symbol == SYM_ACC_FLAT ? acc_above : acc_below;
}

int sheetController::get_rest_symbol(const int noteType) {
  switch (noteType) {
    case NOTE_WHOLE:
      return SYM_REST_WHOLE;
    case NOTE_HALF:
      return SYM_REST_HALF;
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
      return SYM_REST_4;
    case NOTE_8:
      return SYM_REST_8;
    case NOTE_16:
      return SYM_REST_16;
    case NOTE_32:
      return SYM_REST_32;
    default:
      return SYM_REST_64;
  }
}

int sheetController::getSymbolType(const int noteType) {
  switch (noteType) {
    case NOTE_LARGE:
    case NOTE_WHOLE_DOT:
    case NOTE_WHOLE:
      return SYM_HEAD_WHOLE;
    case NOTE_HALF_DOT:
    case NOTE_HALF:
      return SYM_HEAD_HALF;
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
    case NOTE_8_DOT:
    case NOTE_8:
    case NOTE_16_DOT:
    case NOTE_16:
    case NOTE_32:
    case NOTE_64:
      return SYM_HEAD_STD;
    default:
      logQ("invalid note type:", noteType);
      break;
  }

  return -1;
}
