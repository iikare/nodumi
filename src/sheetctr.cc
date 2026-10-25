#include "sheetctr.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <bit>
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

// beats strictly inside a value starting off the beat, which it would hide, or in compound meter (whose beats are
// dotted) inside one starting on the beat that does not last whole beats
int find_hidden_beats(int pos, int units, int beat) {
  const int inside = (pos + units - 1) / beat - pos / beat;
  if (pos % beat != 0) {
    return inside;
  }
  return beat % 3 == 0 && units % beat != 0 ? inside : 0;
}

// splits [begin, end) in 64ths into as few tied values as reads well: extra dots cost a little, a tied piece off
// its own grid costs a piece, and a value hiding the beats it crosses costs most of one per beat
template <typename F>
void split_duration(int begin, int end, int beat, F&& emit) {
  constexpr float dot_cost = 0.3f;
  constexpr float beat_cost = 0.75f;

  // most notes are one value that hides no beat
  const note_value exact = find_exact_value(end - begin);
  if (exact.type != NOTE_NONE && find_hidden_beats(begin, exact.units, beat) == 0) {
    emit(begin, exact.type, exact.dots, true);
    return;
  }

  // cheapest split of each remainder, filled from the end
  const int len = end - begin;
  vector<float> cost(len + 1, std::numeric_limits<float>::max());
  vector<const note_value*> pick(len, nullptr);
  cost[len] = 0;
  for (int i = len - 1; i >= 0; --i) {
    const int pos = begin + i;
    for (const note_value& v : note_values) {
      if (v.units > len - i) {
        continue;
      }
      const int grid = v.dots == 1 ? note_units[v.type] / 2 : note_units[v.type];
      float c = 1 + dot_cost * std::max(0, v.dots - 1) + cost[i + v.units];
      c += i > 0 && pos % grid != 0 ? 1 : 0;
      c += beat_cost * find_hidden_beats(pos, v.units, beat);
      if (c < cost[i] - 0.001f) {
        cost[i] = c;
        pick[i] = &v;
      }
    }
  }

  for (int i = 0; i < len; i += pick[i]->units) {
    emit(begin + i, pick[i]->type, pick[i]->dots, i + pick[i]->units == len);
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

bool sheetController::is_quantized() const {
  return ctr.option.get(ctr.getLiveState() ? OPTION::SET_QUANTIZE_LIVE : OPTION::SET_QUANTIZE_MIDI);
}

int sheetController::find_grid(const midi& stream) const {
  // the grid set for live play or for files, or else the finest one for a file and quarters while playing live
  if (is_quantized()) {
    return ctr.option.get(ctr.getLiveState() ? OPTION::QUANTIZE_LIVE : OPTION::QUANTIZE_MIDI);
  }
  return std::max(1, to_units(stream, ctr.getMinTickLen()));
}

int sheetController::find_note_begin(const midi& stream, const measureController& measure, const note& n) const {
  // snap to the nearest grid position
  const int grid = find_grid(stream);
  const int len = std::max(1, to_units(stream, measure.getTickLen()));
  const double units = (n.tick - measure.getTick()) * 16.0 / std::max(1, stream.getTPQ());
  const int pos = static_cast<int>(std::lround(units / grid)) * grid;
  return std::clamp(pos, 0, (len - 1) / grid * grid);
}

int sheetController::find_value_units(const midi& stream, int ticks, int type) const {
  const int units = find_raw_value_units(stream, ticks, type);
  if (!is_quantized()) {
    return units;
  }
  // a quantized note lasts whole grid steps, so nothing shorter than the grid is written
  const int grid = find_grid(stream);
  return std::max(grid, static_cast<int>(std::lround(static_cast<double>(units) / grid)) * grid);
}

int sheetController::find_raw_value_units(const midi& stream, int ticks, int type) const {
  const double actual = ticks * 16.0 / std::max(1, stream.getTPQ());

  // very short notes keep their rounded-up value
  if (actual <= note_units[NOTE_16_DOT]) {
    return note_units[std::clamp(type, static_cast<int>(NOTE_16_DOT), static_cast<int>(NOTE_64))];
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

int sheetController::find_ledger_count(int sheet_y, int stave) const {
  const int top = stave == STAVE_TREBLE ? 10 : -2;
  const int bottom = stave == STAVE_TREBLE ? 2 : -10;
  if (sheet_y > getStaveRenderLimit().first || sheet_y < getStaveRenderLimit().second) {
    return octave_oob_cost;
  }
  return std::max(0, (sheet_y - top) / 2) + std::max(0, (bottom - sheet_y) / 2);
}

int sheetController::find_octave_y(int sheet_y, int stave, int octave) const {
  // notes under an 8va line sit an octave lower than they sound, under an 8vb line an octave higher
  return sheet_y + (stave == STAVE_TREBLE ? -7 : 7) * octave;
}

void sheetController::find_octaves(sheetMeasure& dm, int stave, int prev, vector<int>& state) const {
  // each chord goes under an 8va or 8vb line or not, whichever needs fewer ledger lines over the measure, where
  // starting or stopping a line and moving each note cost a little, so lines cover passages far off the stave, notes
  // pushed far past the other side cost a lot, and a line never starts or stops while a tied note sounds
  const midi& stream = ctr.getStream();
  const int count = dm.chords.size();
  vector<std::array<float, 2>> cost(count, {0.0f, 0.0f});
  vector<bool> locked(count, false);  // no change before this chord, which for the first is at the barline

  // a stave gathering more parts than two hands, as in an orchestral file, only gets busier with lines coming and
  // going
  vector<int> tracks;
  for (const auto& c : dm.chords) {
    for (const sheetNote* n : c.second) {
      const int track = stream.notes[n->oriNote].track;
      if (n->stave == stave && std::ranges::find(tracks, track) == tracks.end()) {
        tracks.push_back(track);
      }
    }
  }
  const float crowded = tracks.size() > 2 ? octave_far_cost : 0.0f;
  for (int ch = 0; ch < count; ++ch) {
    const auto& c = dm.chords[ch].second;
    for (unsigned int i = 0; i < c.size(); ++i) {
      const sheetNote* n = c[i];
      if (n->stave != stave) {
        continue;
      }
      if (n->tie_in) {
        int from = -1;
        for (int e = ch - 1; e >= 0 && from < 0; --e) {
          if (std::ranges::find(dm.chords[e].second, n->oriNote, &sheetNote::oriNote) != dm.chords[e].second.end()) {
            from = e;
          }
        }
        for (int k = from + 1; k <= ch; ++k) {
          locked[k] = true;
        }
      }

      const int sheet_y = stream.notes[n->oriNote].sheetY;
      if (i > 0 && c[i - 1]->stave == stave && stream.notes[c[i - 1]->oriNote].sheetY == sheet_y) {
        continue;
      }
      const int moved = find_octave_y(sheet_y, stave, 1);
      const int after = find_ledger_count(moved, stave);
      const bool other_side = stave == STAVE_TREBLE ? moved < 2 : moved > -2;
      cost[ch][0] += find_ledger_count(sheet_y, stave);
      cost[ch][1] += after + octave_note_cost + crowded + (other_side && after > octave_far_side ? octave_far_cost : 0);
    }
  }

  // a line never starts or stops inside what will be one beam either, short notes running on within a beat
  const int beat = sheetMeasure::find_beat(dm.measure->currentTime);
  int prev_ch = -1;
  int prev_end = 0;
  for (int ch = 0; ch < count; ++ch) {
    int shortest = std::numeric_limits<int>::max();
    for (const sheetNote* n : dm.chords[ch].second) {
      shortest = n->stave == stave ? std::min(shortest, note_len(n)) : shortest;
    }
    if (shortest == std::numeric_limits<int>::max()) {
      continue;
    }
    const int pos = dm.chords[ch].first;
    const bool beamed = shortest < note_units[NOTE_QUARTER];
    if (beamed && prev_ch >= 0 && prev_end >= pos && dm.chords[prev_ch].first / beat == pos / beat) {
      for (int k = prev_ch + 1; k <= ch; ++k) {
        locked[k] = true;
      }
    }
    prev_ch = beamed ? ch : -1;
    prev_end = pos + shortest;
  }

  // the cheapest state for each chord given the state before it, keeping the state when a change saves nothing
  constexpr float never = 1e9f;
  vector<std::array<float, 2>> best(count);
  vector<std::array<int, 2>> came(count);
  for (int ch = 0; ch < count; ++ch) {
    for (int s = 0; s < 2; ++s) {
      best[ch][s] = never;
      came[ch][s] = s;
      for (int p = 0; p < 2; ++p) {
        const float before = ch == 0 ? (p == prev ? 0.0f : never) : best[ch - 1][p];
        const float change = p == s ? 0.0f : locked[ch] ? never : s == 1 ? octave_start_cost : octave_stop_cost;
        const float total = before + change + cost[ch][s];
        if (total < best[ch][s] - 0.001f || (p == s && total < best[ch][s] + 0.001f)) {
          best[ch][s] = total;
          came[ch][s] = p;
        }
      }
    }
  }

  state.assign(count, prev);
  int s = count > 0 && best[count - 1][1] < best[count - 1][0] - 0.001f ? 1 : count > 0 ? 0 : prev;
  dm.octave_out[stave] = s;
  for (int ch = count - 1; ch >= 0; --ch) {
    state[ch] = s;
    s = came[ch][s];
  }
}

int sheetController::find_note_end(const midi& stream, const note& n) const {
  if (n.measure < 0 || n.measure >= static_cast<int>(stream.measureMap.size())) {
    return 0;
  }
  const measureController& measure = stream.measureMap[n.measure];
  return to_units(stream, measure.getTick()) + find_note_begin(stream, measure, n) +
         find_value_units(stream, n.tickDuration, n.type);
}

bool sheetController::is_grace_length(int idx) const {
  const midi& stream = ctr.getStream();
  const note& n = stream.notes[idx];
  return n.tickDuration * 16.0 / std::max(1, stream.getTPQ()) <= grace_max_units && n.duration <= grace_max_time;
}

int sheetController::find_next_struck(int idx) const {
  // the next note of the same track within an octave, when it follows straight on
  const midi& stream = ctr.getStream();
  const note& n = stream.notes[idx];
  int next = -1;
  for (int m = std::max(0, n.measure); m <= n.measure + 1 && m < static_cast<int>(stream.measureMap.size()); ++m) {
    for (int j : stream.measureMap[m].notes) {
      const note& o = stream.notes[j];
      if (o.track == n.track && o.x > n.x + grace_min_lead && std::abs(o.y - n.y) <= 12 &&
          (next < 0 || o.x < stream.notes[next].x)) {
        next = j;
      }
    }
  }
  if (next < 0) {
    return -1;
  }
  const note& o = stream.notes[next];
  return o.x - (n.x + n.duration) <= grace_join && o.x - n.x <= grace_max_lead ? next : -1;
}

int sheetController::find_prev_struck(int idx) const {
  // the note this one follows straight on from, the reverse of find_next_struck
  const midi& stream = ctr.getStream();
  const note& n = stream.notes[idx];
  int prev = -1;
  for (int m = std::max(0, n.measure - 1); m <= n.measure && m < static_cast<int>(stream.measureMap.size()); ++m) {
    for (int j : stream.measureMap[m].notes) {
      const note& o = stream.notes[j];
      if (o.track == n.track && o.x < n.x - grace_min_lead && std::abs(o.y - n.y) <= 12 &&
          (prev < 0 || o.x > stream.notes[prev].x)) {
        prev = j;
      }
    }
  }
  return prev >= 0 && find_next_struck(prev) == idx ? prev : -1;
}

int sheetController::find_grace_main(int idx) const {
  // a very short note running straight into a clearly longer one nearby is a grace note, and so are up to two more
  // before it, but a longer run of short notes is written out, so the first of them must not follow another
  if (!is_grace_length(idx)) {
    return -1;
  }
  int count = 1;
  for (int prev = find_prev_struck(idx); prev >= 0 && is_grace_length(prev); prev = find_prev_struck(prev)) {
    if (++count > grace_max_count) {
      return -1;
    }
  }
  int main = find_next_struck(idx);
  for (; main >= 0 && is_grace_length(main); main = find_next_struck(main)) {
    if (++count > grace_max_count) {
      return -1;
    }
  }
  if (main < 0) {
    return -1;
  }

  const midi& stream = ctr.getStream();
  const int sixteenth = stream.getTPQ() / 4;
  const int shortest = std::max(sixteenth, grace_main_ratio * stream.notes[idx].tickDuration);
  return stream.notes[main].tickDuration >= shortest ? main : -1;
}

void sheetController::find_rolls(vector<note_onset>& onsets) const {
  // notes struck in quick succession and left ringing together are one chord, rolled when it rises steadily and
  // spreads far enough to be heard that way, and otherwise only played loosely, which is looked for across all tracks
  // first, as a roll can pass from one hand to the other, and then in each track, as both hands can roll at once
  const midi& stream = ctr.getStream();
  int rolls = 0;
  vector<int> order;
  auto scan = [&](bool by_track) {
    order.clear();
    for (unsigned int k = 0; k < onsets.size(); ++k) {
      if (onsets[k].end < 0) {
        order.push_back(k);
      }
    }
    std::ranges::sort(order, [&](int a, int b) {
      const note& na = stream.notes[onsets[a].note];
      const note& nb = stream.notes[onsets[b].note];
      if (by_track && na.track != nb.track) {
        return na.track < nb.track;
      }
      return na.x != nb.x ? na.x < nb.x : na.y < nb.y;
    });
    auto at = [&](unsigned int k) -> const note& { return stream.notes[onsets[order[k]].note]; };

    for (unsigned int i = 0; i < order.size();) {
      unsigned int j = i + 1;
      while (j < order.size() && (!by_track || at(j).track == at(i).track) && at(j).x - at(j - 1).x <= roll_step &&
             at(j).x - at(i).x <= roll_span) {
        ++j;
      }

      // notes struck almost together are one strike, and a roll climbs through at least three
      const double spread = at(j - 1).x - at(i).x;
      bool rings = j - i >= 2;
      bool rising = true;
      bool apart = false;
      int strikes = 1;
      int strike_lo = at(i).y;
      int prev_hi = std::numeric_limits<int>::min();
      int strike_hi = at(i).y;
      for (unsigned int k = i; k < j; ++k) {
        rings = rings && at(k).x + at(k).duration >= at(j - 1).x + std::max(roll_hold, spread);
        apart = apart || onsets[order[k]].begin != onsets[order[i]].begin;
        if (k > i && at(k).x - at(k - 1).x >= roll_min_step) {
          rising = rising && strike_lo > prev_hi;
          prev_hi = strike_hi;
          strike_lo = at(k).y;
          strike_hi = at(k).y;
          ++strikes;
        }
        strike_lo = std::min(strike_lo, at(k).y);
        strike_hi = std::max(strike_hi, at(k).y);
      }
      rising = rising && strike_lo > prev_hi;
      const bool rolled = rings && rising && strikes >= 3 && spread >= roll_min_spread;
      // a chord played loosely is close in time, or a little further when that is still within a 32nd
      int first_begin = onsets[order[i]].begin;
      int last_begin = first_begin;
      for (unsigned int k = i; k < j; ++k) {
        first_begin = std::min(first_begin, onsets[order[k]].begin);
        last_begin = std::max(last_begin, onsets[order[k]].begin);
      }
      const bool near = spread <= chord_spread || (spread <= chord_spread_far && last_begin - first_begin <= 2);
      const bool loose = rings && apart && near;
      if (!rolled && !loose) {
        // a shorter run from a later note may still be one
        i += rings ? j - i : 1;
        continue;
      }

      // every note starts with the first and lasts as if struck with it
      for (unsigned int k = i; k < j; ++k) {
        note_onset& o = onsets[order[k]];
        o.begin = onsets[order[i]].begin;
        o.end = o.begin + find_value_units(stream, at(k).tick + at(k).tickDuration - at(i).tick, at(k).type);
        o.arpeggio = rolled ? rolls : -1;
      }
      rolls += rolled;
      i = j;
    }
  };
  scan(false);
  scan(true);
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
  const int beat = sheetMeasure::find_beat(measure.currentTime);

  measure.displayNotes.clear();

  // grace notes lead into a note of this measure from this measure or the end of the one before, and take no time,
  // so they are kept apart from the notes that do
  const int index = measure.notes.empty() ? -1 : stream.notes[measure.notes.front()].measure;
  vector<pair<int, int>> graces;  // grace note and the note it leads into
  vector<note_onset> onsets;
  for (int idx : measure.notes) {
    const int main = find_grace_main(idx);
    if (main < 0) {
      onsets.push_back({idx, find_note_begin(stream, measure, stream.notes[idx])});
    }
    else if (stream.notes[main].measure == index) {
      graces.push_back(make_pair(idx, main));
    }
  }
  if (index > 0) {
    for (int idx : stream.measureMap[index - 1].notes) {
      const int main = find_grace_main(idx);
      if (main >= 0 && stream.notes[main].measure == index) {
        graces.push_back(make_pair(idx, main));
      }
    }
  }
  std::ranges::sort(graces,
                    [&](const auto& a, const auto& b) { return stream.notes[a.first].x < stream.notes[b.first].x; });
  find_rolls(onsets);

  // a grace note leading into a rolled chord is where the roll begins, so it joins the chord instead
  std::erase_if(graces, [&](const pair<int, int>& grace) {
    auto it = std::ranges::find(onsets, grace.second, &note_onset::note);
    if (it == onsets.end() || it->arpeggio < 0) {
      return false;
    }
    const note_onset joined = {grace.first, it->begin, it->end, it->arpeggio};
    onsets.push_back(joined);
    return true;
  });

  // a grace note on a stronger beat than the note it leads into brings that note's chord back onto it, keeping their
  // values
  auto find_weight = [](int pos) { return pos == 0 ? 32 : std::countr_zero(static_cast<unsigned int>(pos)); };
  for (const auto& [grace, main] : graces) {
    auto it = std::ranges::find(onsets, main, &note_onset::note);
    if (stream.notes[grace].measure != index || it == onsets.end()) {
      continue;
    }
    const int from = find_note_begin(stream, measure, stream.notes[grace]);
    const int to = it->begin;
    if (from < to && find_weight(from) > find_weight(to)) {
      for (auto& o : onsets) {
        const note& n = stream.notes[o.note];
        if (o.begin == to && n.track == stream.notes[main].track) {
          o.end = (o.end >= 0 ? o.end - to : find_value_units(stream, n.tickDuration, n.type)) + from;
          o.begin = from;
        }
      }
    }
  }

  // where the same key is next played in this measure
  auto find_restrike = [&](int idx, int begin) {
    const note& src = stream.notes[idx];
    int next = len;
    for (const auto& o : onsets) {
      const note& m = stream.notes[o.note];
      if (o.begin > begin && m.y == src.y && m.track == src.track) {
        next = std::min(next, o.begin);
      }
    }
    return next;
  };

  auto add_note = [&](const note_onset& o, bool tie_in) {
    const int idx = o.note;
    const int begin = o.begin;
    const bool tie_out = std::ranges::find(measure.tie_out_notes, idx) != measure.tie_out_notes.end();
    int end = tie_out ? len : std::min(len, o.end >= 0 ? o.end : find_note_end(stream, stream.notes[idx]) - bar_begin);
    // a key played again well before a note's rounded end cuts that note short, on the sixteenth grid if it can
    const int restrike = find_restrike(idx, begin);
    if (!tie_out && end - restrike >= legato_units) {
      const int snapped = restrike - restrike % note_units[NOTE_16];
      end = snapped > begin ? snapped : restrike;
    }
    // a held live note may have only just crossed the barline
    end = std::min(len, std::max(end, begin + (tie_in ? note_units[NOTE_16] : 1)));

    split_duration(begin, end, beat, [&](int pos, int type, int dots, bool last) {
      measure.displayNotes.push_back({pos, ACC_NONE, STAVE_NONE, true, true, false, idx, type, dots,
                                      tie_in || pos != begin, tie_out || !last, false, 0, false, 0,
                                      stream.notes[idx].sheetY, pos == begin && !tie_in ? o.arpeggio : -1, -1});
    });
  };

  for (const auto& o : onsets) {
    add_note(o, false);
  }
  for (int idx : measure.tie_in_notes) {
    add_note({idx, 0}, true);
  }

  // a lone grace note is an eighth and a group are sixteenths, all placed where their note begins
  for (const auto& [grace, main] : graces) {
    auto it = std::ranges::find(onsets, main, &note_onset::note);
    if (it == onsets.end()) {
      continue;
    }
    const int count = std::ranges::count(graces, main, &pair<int, int>::second);
    measure.displayNotes.push_back({it->begin, ACC_NONE, STAVE_NONE, true, true, false, grace,
                                    count == 1 ? NOTE_8 : NOTE_16, 0, false, false, false, 0, false, 0,
                                    stream.notes[grace].sheetY, -1, main});
  }
}

void sheetController::disectMeasure(measureController& measure) {
  // two voices whose stems run into the other stave are no clearer than one, so a crowded measure is laid out again
  // with a stave in one voice, first the one whose voices sit closer together and both only when needed
  std::array<bool, 2> chosen = {false, false};
  int crowded[2] = {0, 0};
  sheetMeasure dm = layout_measure(measure, chosen, crowded);
  int best = crowded[STAVE_TREBLE] + crowded[STAVE_BASS];

  if (best > 0) {
    const bool split[2] = {dm.two_voice[STAVE_TREBLE], dm.two_voice[STAVE_BASS]};
    const bool treble_first = dm.voice_gap[STAVE_TREBLE] <= dm.voice_gap[STAVE_BASS];
    const std::array<bool, 2> options[3] = {{treble_first, !treble_first}, {!treble_first, treble_first}, {true, true}};
    std::array<bool, 2> last = chosen;
    for (const auto& option : options) {
      if ((option[STAVE_TREBLE] && !split[STAVE_TREBLE]) || (option[STAVE_BASS] && !split[STAVE_BASS])) {
        continue;
      }
      int trial[2] = {0, 0};
      dm = layout_measure(measure, option, trial);
      last = option;
      if (trial[STAVE_TREBLE] + trial[STAVE_BASS] < best) {
        best = trial[STAVE_TREBLE] + trial[STAVE_BASS];
        chosen = option;
      }
      if (best == 0) {
        break;
      }
    }
    // the notes are rebuilt on every layout, so the kept one has to be the last made
    if (last != chosen) {
      dm = layout_measure(measure, chosen, crowded);
    }
  }
  displayMeasure.push_back(std::move(dm));
}

sheetMeasure sheetController::layout_measure(measureController& measure, const std::array<bool, 2>& one_voice,
                                             int (&crowded)[2]) {
  const midi& stream = ctr.getStream();

  build_display_notes(measure);

  sheetMeasure dm;
  dm.setParent(measure);
  dm.one_voice[STAVE_TREBLE] = one_voice[STAVE_TREBLE];
  dm.one_voice[STAVE_BASS] = one_voice[STAVE_BASS];
  dm.buildChordMap(measure.displayNotes);

  // each note goes on the stave of its pitch, a middle c by its place in the chord
  for (const auto& c : dm.chords) {
    for (unsigned int ct = 0; const auto& n : c.second) {
      const note& src = stream.notes[n->oriNote];
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

  // a low part under clearly higher treble notes, with nothing close above it at the time or near it a sixteenth
  // either side, goes down to the bass stave when nothing there reaches within a third of it, a chord's low notes
  // all together
  vector<sheetNote*> absorbed;
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    const int begin = dm.chords[ch].first;
    bool rejected = false;
    for (sheetNote* n : dm.chords[ch].second) {
      const int sheet_y = stream.notes[n->oriNote].sheetY;
      if (n->stave != STAVE_TREBLE || n->tie_in || sheet_y > absorb_top) {
        continue;
      }
      const int end = begin + note_len(n);
      bool above = false;
      bool blocked = false;
      for (const auto& c : dm.chords) {
        for (const sheetNote* m : c.second) {
          const int y = stream.notes[m->oriNote].sheetY;
          const int m_end = c.first + note_len(m);
          const int near = note_units[NOTE_16];
          if (m->oriNote == n->oriNote || c.first >= end + near || m_end + near <= begin) {
            continue;
          }
          const bool during = c.first < end && m_end > begin;
          if (m->stave == STAVE_TREBLE) {
            above = above || (during && y >= sheet_y + absorb_gap);
            blocked = blocked || (y > absorb_top && y < sheet_y + (during ? absorb_gap : absorb_near));
          }
          else {
            blocked = blocked || (during && y > sheet_y - 2);
          }
        }
      }
      if (above && !blocked) {
        absorbed.push_back(n);
      }
      else {
        rejected = true;
      }
    }
    if (rejected) {
      std::erase_if(absorbed, [&](const sheetNote* m) { return m->displayBegin == begin; });
    }
  }
  for (sheetNote* n : absorbed) {
    n->stave = STAVE_BASS;
  }

  // a tie never switches staves, so a continued note stays where it started
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (sheetNote* n : dm.chords[ch].second) {
      const note& src = stream.notes[n->oriNote];
      if (!n->tie_in) {
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

  // notes rendering on the same position share a stave
  for (const auto& c : dm.chords) {
    for (unsigned int ct = 1; ct < c.second.size(); ++ct) {
      if (stream.notes[c.second[ct]->oriNote].sheetY == stream.notes[c.second[ct - 1]->oriNote].sheetY) {
        c.second[ct]->stave = c.second[ct - 1]->stave;
      }
    }
  }

  // a grace note goes with the note it leads into
  auto find_main = [&](const sheetNote* g) -> sheetNote* {
    for (const auto& c : dm.chords) {
      for (sheetNote* n : c.second) {
        if (c.first == g->displayBegin && n->oriNote == g->grace_of && !n->tie_in) {
          return n;
        }
      }
    }
    return nullptr;
  };
  for (sheetNote* g : dm.graces) {
    const sheetNote* main = find_main(g);
    g->stave = main ? main->stave : (stream.notes[g->oriNote].sheetY >= 0 ? STAVE_TREBLE : STAVE_BASS);
  }

  // a stave far from its lines moves an octave nearer under an 8va or 8vb line, and notes still too far away get
  // markers instead, though they keep their stave for rests
  const sheetMeasure* prev = displayMeasure.empty() ? nullptr : &displayMeasure.back();
  vector<int> octave[2];
  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    find_octaves(dm, stave, prev ? prev->octave_out[stave] : 0, octave[stave]);
  }
  auto place = [&](sheetNote* n, int ch) {
    n->sheet_y = find_octave_y(stream.notes[n->oriNote].sheetY, n->stave, ch < 0 ? 0 : octave[n->stave][ch]);
    n->out_of_bounds = n->sheet_y > getStaveRenderLimit().first || n->sheet_y < getStaveRenderLimit().second;
  };
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (sheetNote* n : dm.chords[ch].second) {
      place(n, ch);
    }
  }
  for (sheetNote* g : dm.graces) {
    auto it = std::ranges::find(dm.chords, g->displayBegin, &pair<int, vector<sheetNote*>>::first);
    place(g, it == dm.chords.end() ? -1 : it - dm.chords.begin());
  }

  // run a DFA for each valid midi key, advancing state based on
  // new-note-to-midi-position values each DFA set is unique to its measure add
  // "1" to size for inclusive bounding
  vector<int> presentDFAState;
  findDFAStartVector(presentDFAState, measure.currentKey);

  // find true accidental type for each note, where tied continuations keep the accidental shown on their first piece
  // and grace notes come before their chord
  auto read_acc = [&](sheetNote* n) {
    const note& src = stream.notes[n->oriNote];
    if (!n->out_of_bounds && !n->tie_in) {
      n->displayAcc = getDisplayAccType(presentDFAState[mapSheetY(src.sheetY)], src.accType);
    }
  };
  auto grace = dm.graces.begin();
  for (const auto& c : dm.chords) {
    for (; grace != dm.graces.end() && (*grace)->displayBegin <= c.first; ++grace) {
      read_acc(*grace);
    }
    std::ranges::for_each(c.second, read_acc);
  }

  const int len = std::max(1, to_units(stream, measure.getTickLen()));
  dm.find_voices(len);
  for (sheetNote* g : dm.graces) {
    const sheetNote* main = find_main(g);
    g->voice = main ? main->voice : 0;
  }

  // a duplicate within one voice is hidden behind the first, which is the continuing tie if there is one
  for (const auto& c : dm.chords) {
    for (unsigned int ct = 1; ct < c.second.size(); ++ct) {
      sheetNote* n = c.second[ct];
      for (unsigned int prev = 0; prev < ct; ++prev) {
        const sheetNote* m = c.second[prev];
        if (m->visible && m->voice == n->voice &&
            stream.notes[m->oriNote].sheetY == stream.notes[n->oriNote].sheetY) {
          n->visible = false;
          break;
        }
      }
    }
  }

  const bool open = ctr.getLiveState() && &measure == &stream.measureMap.back();
  dm.close_gaps(len, open);
  dm.find_rests(len, open);

  dm.build_stem_groups(len);
  place_rests(dm);
  dm.build_beams();
  dm.set_stem_dirs();
  keep_in_sheet(dm);
  separate_staves(dm, crowded);
  avoid_ties(dm);
  dm.place_heads();

  find_chord_widths(dm);
  find_spacing(dm);

  return dm;
}

float sheetController::find_sheet_top() const { return ctr.menuHeight + sheet_margin; }

float sheetController::find_sheet_bottom() const {
  // the sheet's height is fixed even while it is hidden, so layout does not depend on it being shown
  return ctr.menuHeight + ctr.menuHeight + ctr.sheetHeight - sheet_margin;
}

void sheetController::place_rests(sheetMeasure& dm) {
  for (auto& cd : dm.s_chordData) {
    for (int part = 0; part < part_count; ++part) {
      const int stave = part_stave(part);
      const int type = cd.rests[part];
      if (type < 0 || !dm.two_voice[stave] || cd.rest_shared[stave]) {
        continue;
      }

      // a voice's rest moves towards its side of the stave and on past the other voice's notes,
      // whole and half rests staying on a line
      const bool upper = part_voice(part) == 0;
      const stem_group& other = cd.groups[find_part(stave, upper ? 1 : 0)];
      int shift = upper ? (type == NOTE_WHOLE || type == NOTE_HALF ? 2 : 4) : -4;
      bool clear = other.empty();
      for (int step = 0; !clear && step < rest_shift_max; ++step) {
        const float y = find_rest_y(type, stave, shift);
        clear = upper ? y + find_rest_extent(type, true) <= findStaveY(other.hi, stave) - head_half - rest_clearance
                      : y - find_rest_extent(type, false) >= findStaveY(other.lo, stave) + head_half + rest_clearance;
        if (!clear) {
          shift += upper ? 2 : -2;
        }
      }

      // a rest with no room, or only room out in the gap between the staves, is left out
      const float y = find_rest_y(type, stave, shift);
      const bool in_gap = stave == STAVE_TREBLE ? !upper && y + find_rest_extent(type, true) > findStaveY(-3, stave)
                                                : upper && y - find_rest_extent(type, false) < findStaveY(3, stave);
      if (!clear || in_gap) {
        cd.rests[part] = -1;
      }
      else {
        cd.rest_shift[part] = shift;
      }
    }
  }

  // a column that only held those rests goes too
  for (int ch = static_cast<int>(dm.chords.size()) - 1; ch >= 0; --ch) {
    const sheetChordData& cd = dm.s_chordData[ch];
    if (dm.chords[ch].second.empty() && std::ranges::all_of(cd.rests, [](int r) { return r < 0; })) {
      dm.chords.erase(dm.chords.begin() + ch);
      dm.s_chordData.erase(dm.s_chordData.begin() + ch);
    }
  }
}

float sheetController::find_other_voice_bound(const sheetMeasure& dm, int first, int last, int part, int dir) {
  // the nearest edge of the other voice's notes and rests in these chords, which a stem pointing at it stops short of
  const int stave = part_stave(part);
  const bool down = dir == FLAG_DOWN;
  float bound = down ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
  for (int ch = first; ch <= last; ++ch) {
    const sheetChordData& cd = dm.s_chordData[ch];
    const stem_group& other = cd.groups[part ^ 1];
    const int rest = cd.rests[part ^ 1];
    const int rest_shift = cd.rest_shift[part ^ 1];
    if (!other.empty()) {
      bound = down ? std::min<float>(bound, findStaveY(other.hi, stave) - head_half)
                   : std::max<float>(bound, findStaveY(other.lo, stave) + head_half);
    }
    if (rest >= 0) {
      const float y = find_rest_y(rest, stave, rest_shift);
      bound = down ? std::min(bound, y - find_rest_extent(rest, false))
                   : std::max(bound, y + find_rest_extent(rest, true));
    }
  }
  return bound;
}

bool sheetController::clears_other_voice(const sheetMeasure& dm, int first, int last, int part, int dir, int lo, int hi,
                                         float len) {
  // a stem or beam turned towards the other voice of its stave must stop short of that voice's notes and rests in
  // every chord it reaches over, its own and those between
  const int stave = part_stave(part);
  const float bound = find_other_voice_bound(dm, first, last, part, dir);
  if (dir == FLAG_DOWN) {
    return findStaveY(lo, stave) + len + stave_clearance <= bound;
  }
  return findStaveY(hi, stave) - len - stave_clearance >= bound;
}

bool sheetController::is_under_line(const sheetMeasure& dm, int first, int last, int part) const {
  for (int ch = first; ch <= last; ++ch) {
    for (const sheetNote* n : dm.chords[ch].second) {
      if (in_part(n, part) && is_moved(n)) {
        return true;
      }
    }
  }
  return false;
}

float sheetController::find_stem_edge(const sheetMeasure& dm, int first, int last, int part, bool top) const {
  // stems and beams reach the edge of the sheet, or stop short of it to leave room for an octave line over them
  const bool room = (part_stave(part) == STAVE_TREBLE) == top && is_under_line(dm, first, last, part);
  return top ? find_sheet_top() + (room ? octave_room : 0) : find_sheet_bottom() - (room ? octave_room : 0);
}

void sheetController::keep_in_sheet(sheetMeasure& dm) {
  // stems and beams never leave the sheet, so one that would turns around when the other way fits
  auto turn = [&](int dir, int lo, int hi, int first, int last, int part, float len) {
    const int stave = part_stave(part);
    const float up_tip = findStaveY(hi, stave) - len;
    const float down_tip = findStaveY(lo, stave) + len;
    const float top = find_stem_edge(dm, first, last, part, true);
    const float bottom = find_stem_edge(dm, first, last, part, false);
    if (dir == FLAG_UP && up_tip < top && down_tip <= bottom) {
      return static_cast<int>(FLAG_DOWN);
    }
    if (dir == FLAG_DOWN && down_tip > bottom && up_tip >= top) {
      return static_cast<int>(FLAG_UP);
    }
    return dir;
  };

  // a voice turns around only when its stems, shortened as far as they may go, then stop short of the other voice,
  // and otherwise drawing shortens them against the edge of the sheet
  auto stops_short = [&](int first, int last, int part, int dir, int lo, int hi, float len, float give, float& limit) {
    const int stave = part_stave(part);
    limit = find_other_voice_bound(dm, first, last, part, dir) + (dir == FLAG_DOWN ? -1 : 1) * stave_clearance;
    return dir == FLAG_DOWN ? findStaveY(lo, stave) + len - give <= limit : findStaveY(hi, stave) - len + give >= limit;
  };

  for (auto& b : dm.beams) {
    int lo = std::numeric_limits<int>::max();
    int hi = std::numeric_limits<int>::min();
    int levels = 0;
    for (int ch : b.chords) {
      const stem_group& g = dm.s_chordData[ch].groups[b.part()];
      lo = std::min(lo, g.stem_lo);
      hi = std::max(hi, g.stem_hi);
      levels = std::max(levels, g.flag_type - FLAGTYPE_8);
    }
    const float len = beam_stem_len + levels * beam_step + beam_thickness;
    const int dir = turn(b.dir, lo, hi, b.chords.front(), b.chords.back(), b.part(), len);
    float limit = 0;
    if (dir == b.dir ||
        (b.fixed && !stops_short(b.chords.front(), b.chords.back(), b.part(), dir, lo, hi, len,
                                 beam_stem_len - beam_min_len, limit))) {
      continue;
    }
    b.dir = dir;
    b.limited = b.fixed;
    b.limit = limit;
    for (int ch : b.chords) {
      dm.s_chordData[ch].groups[b.part()].dir = b.dir;
    }
  }

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (int part = 0; part < part_count; ++part) {
      stem_group& g = dm.s_chordData[ch].groups[part];
      if (g.empty() || !g.has_stem() || g.beam >= 0) {
        continue;
      }
      const float len = stem_len + (g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0);
      const int dir = turn(g.dir, g.stem_lo, g.stem_hi, ch, ch, part, len);
      float limit = 0;
      if (dir == g.dir) {
        continue;
      }
      if (!g.fixed) {
        g.dir = dir;
      }
      else if (stops_short(ch, ch, part, dir, g.stem_lo, g.stem_hi, len, stem_len - stem_min_len, limit)) {
        g.dir = dir;
        g.limited = true;
        g.limit = limit;
      }
    }
  }
}

void sheetController::separate_staves(sheetMeasure& dm, int (&crowded)[2]) {
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
    int part;
    bool fixed;  // a voice's direction, which pulls back and only turns when that clears its other voice
    bool flag;   // a lone stem's flag, which reaches past the stem
    int lo;
    int hi;
    float len;
    bool done;
  };

  vector<stem_unit> units[2];
  auto sheet_top = [&](const stem_unit& u) { return find_stem_edge(dm, u.first, u.last, u.part, true); };
  auto sheet_bottom = [&](const stem_unit& u) { return find_stem_edge(dm, u.first, u.last, u.part, false); };

  auto add_unit = [&](int part, int first, int last, int beam, int ch, bool fixed, const auto& groups) {
    const int stave = part_stave(part);
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
    // a voice's lone stem cannot turn around, so it may shorten instead
    const float give = beam >= 0 ? beam_stem_len - beam_min_len : (fixed ? stem_len - stem_min_len : 0);
    const bool flag = beam < 0 && groups.size() == 1 && groups[0]->has_flag();
    units[stave].push_back({first, last, treble ? down_tip : up_tip, treble ? up_tip : down_tip, give,
                            std::abs((hi - mid) - (mid - lo)), beam, ch, part, fixed, flag, lo, hi, len, false});
  };

  // treble stems pointing down sit left of their notes and bass stems pointing up right of them,
  // so within one chord they only meet through a treble flag
  auto meets = [](const stem_unit& t, const stem_unit& b) {
    const int t0 = 2 * t.first;
    const int t1 = 2 * t.last + (t.flag ? 1 : 0);
    const int b0 = 2 * b.first + 1;
    const int b1 = 2 * b.last + 1;
    return t0 <= b1 && b0 <= t1;
  };

  for (unsigned int b = 0; b < dm.beams.size(); ++b) {
    const beam_group& beam = dm.beams[b];
    if (beam.dir != (beam.stave == STAVE_TREBLE ? FLAG_DOWN : FLAG_UP)) {
      continue;
    }
    vector<const stem_group*> groups;
    for (int ch : beam.chords) {
      groups.push_back(&dm.s_chordData[ch].groups[beam.part()]);
    }
    add_unit(beam.part(), beam.chords.front(), beam.chords.back(), b, -1, beam.fixed, groups);
  }

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (int part = 0; part < part_count; ++part) {
      const stem_group& g = dm.s_chordData[ch].groups[part];
      const int toward_gap = part_stave(part) == STAVE_TREBLE ? FLAG_DOWN : FLAG_UP;
      if (!g.empty() && g.has_stem() && g.beam < 0 && g.dir == toward_gap) {
        add_unit(part, ch, ch, -1, ch, g.fixed, std::array<const stem_group*, 1>{&g});
      }
    }
  }

  // turning around is only an option when the stems still fit on the sheet
  auto fits_flipped = [&](const stem_unit& u, int stave) {
    return !u.fixed && (stave == STAVE_TREBLE ? u.flipped_tip >= sheet_top(u) : u.flipped_tip <= sheet_bottom(u));
  };
  // a voice may still turn away from the gap when that keeps it clear of the other voice on its own stave
  auto turns_aside = [&](const stem_unit& u, int stave) {
    const bool fits = stave == STAVE_TREBLE ? u.flipped_tip >= sheet_top(u) : u.flipped_tip <= sheet_bottom(u);
    if (!u.fixed || !fits) {
      return false;
    }
    const int dir = stave == STAVE_TREBLE ? FLAG_UP : FLAG_DOWN;
    return clears_other_voice(dm, u.first, u.last, u.part, dir, u.lo, u.hi, u.len);
  };
  auto flip = [&](stem_unit& u, int stave) {
    const int dir = stave == STAVE_TREBLE ? FLAG_UP : FLAG_DOWN;
    if (u.beam >= 0) {
      dm.beams[u.beam].dir = dir;
      dm.beams[u.beam].limited = false;
      for (int ch : dm.beams[u.beam].chords) {
        dm.s_chordData[ch].groups[u.part].dir = dir;
      }
    }
    else {
      dm.s_chordData[u.ch].groups[u.part].dir = dir;
    }
    u.done = true;
  };
  auto pull_back = [&](const stem_unit& u, float limit) {
    if (u.beam >= 0) {
      dm.beams[u.beam].limited = true;
      dm.beams[u.beam].limit = limit;
    }
    else if (u.fixed) {
      dm.s_chordData[u.ch].groups[u.part].limited = true;
      dm.s_chordData[u.ch].groups[u.part].limit = limit;
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
      else if (turns_aside(b, STAVE_BASS)) {
        flip(b, STAVE_BASS);
      }
      else if (turns_aside(t, STAVE_TREBLE)) {
        flip(t, STAVE_TREBLE);
      }
      else {
        // neither can turn without leaving the sheet, so both pull back from the middle of the gap
        pull_back(t, gap_mid - stave_clearance / 2);
        pull_back(b, gap_mid + stave_clearance / 2);
        // voices that still meet after shortening as far as they can count against their staves
        const float t_tip = std::max(t.tip - t.give, gap_mid - stave_clearance / 2);
        const float b_tip = std::min(b.tip + b.give, gap_mid + stave_clearance / 2);
        if (meets(t, b) && b_tip - t_tip < stave_clearance - 0.5f) {
          crowded[STAVE_TREBLE] += t.fixed;
          crowded[STAVE_BASS] += b.fixed;
        }
      }
    }
  }

  // noteheads, rests and stave lines cannot move, so a stem reaching them pulls back or turns around,
  // and a voice still reaching the other stave's notes, rests or ties counts against its stave
  auto resolve = [&](stem_unit& u, int stave, float bound, float content) {
    const bool treble = stave == STAVE_TREBLE;
    const float overlap = treble ? u.tip + stave_clearance - bound : bound - (u.tip - stave_clearance);
    if (overlap <= 0) {
      return;
    }
    const float limit = treble ? bound - stave_clearance : bound + stave_clearance;
    if (overlap > u.give && turns_aside(u, stave)) {
      flip(u, stave);
    }
    else if (overlap <= u.give || !fits_flipped(u, stave)) {
      pull_back(u, limit);
      const float reach = treble ? u.tip + stave_clearance - content : content - (u.tip - stave_clearance);
      crowded[stave] += u.fixed && reach > u.give;
    }
    else {
      flip(u, stave);
    }
  };

  // an outer tie bends away from its chord, so it can reach into the gap as well
  auto find_tie_reach = [&](int stave, int first, int last, float bound) {
    const bool treble = stave == STAVE_TREBLE;
    for (int c = 0; c <= last && c < static_cast<int>(dm.chords.size()); ++c) {
      for (int voice = 0; voice < 2; ++voice) {
        const int part = find_part(stave, voice);
        const stem_group& g = dm.s_chordData[c].groups[part];
        if (g.empty()) {
          continue;
        }
        const auto& notes = dm.chords[c].second;
        const sheetNote* n = notes[treble ? g.first : g.last];
        if (!n->tie_in && !n->tie_out) {
          continue;
        }

        // in two voices only the voice nearer the gap ties into it
        if (dm.two_voice[stave] && voice == (treble ? 0 : 1)) {
          continue;
        }
        // a lone note ties away from its stem, the outer notes of a chord away from the chord
        int count = 0;
        for (int i = g.first; i <= g.last; ++i) {
          count += in_part(notes[i], part);
        }
        if (!dm.two_voice[stave] && count == 1 && g.dir != (treble ? FLAG_UP : FLAG_DOWN)) {
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

        const float y = findStaveY(n->sheet_y, stave);
        bound = treble ? std::max(bound, y + tie_outside_offset + tie_max_height)
                       : std::min(bound, y - tie_outside_offset - tie_max_height);
      }
    }
    return bound;
  };

  for (auto& b : units[STAVE_BASS]) {
    if (b.done) {
      continue;
    }
    float floor = find_tie_reach(STAVE_TREBLE, b.first, b.last, std::numeric_limits<float>::lowest());
    for (int ch = b.first; ch <= b.last; ++ch) {
      const sheetChordData& cd = dm.s_chordData[ch];
      for (int voice = 0; voice < 2; ++voice) {
        const int part = find_part(STAVE_TREBLE, voice);
        if (!cd.groups[part].empty()) {
          floor = std::max(floor, findStaveY(cd.groups[part].lo, STAVE_TREBLE) + head_half);
        }
        if (cd.rests[part] >= 0) {
          floor = std::max(floor, find_rest_y(cd.rests[part], STAVE_TREBLE, cd.rest_shift[part]) +
                                      find_rest_extent(cd.rests[part], true));
        }
      }
    }
    resolve(b, STAVE_BASS, std::max<float>(floor, findStaveY(2, STAVE_TREBLE)), floor);
  }
  for (auto& t : units[STAVE_TREBLE]) {
    if (t.done) {
      continue;
    }
    float ceiling = find_tie_reach(STAVE_BASS, t.first, t.last, std::numeric_limits<float>::max());
    for (int ch = t.first; ch <= t.last; ++ch) {
      const sheetChordData& cd = dm.s_chordData[ch];
      for (int voice = 0; voice < 2; ++voice) {
        const int part = find_part(STAVE_BASS, voice);
        if (!cd.groups[part].empty()) {
          ceiling = std::min(ceiling, findStaveY(cd.groups[part].hi, STAVE_BASS) - head_half);
        }
        if (cd.rests[part] >= 0) {
          ceiling = std::min(ceiling, find_rest_y(cd.rests[part], STAVE_BASS, cd.rest_shift[part]) -
                                          find_rest_extent(cd.rests[part], false));
        }
      }
    }
    resolve(t, STAVE_TREBLE, std::min<float>(ceiling, findStaveY(-2, STAVE_BASS)), ceiling);
  }
}

void sheetController::avoid_ties(sheetMeasure& dm) {
  // a tie from an earlier chord runs under or over later ones, so stems crossing it turn away instead
  const int chord_count = dm.chords.size();

  struct tie_span {
    int start;
    int end;
    float y;  // the tied note, the tie bends away from it
  };

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    // voices keep their directions
    if (dm.two_voice[stave]) {
      continue;
    }
    const int part = find_part(stave, 0);

    // only turning away from the gap between the staves, so the staves stay apart
    const bool treble = stave == STAVE_TREBLE;
    const int toward_gap = treble ? FLAG_DOWN : FLAG_UP;

    // outer ties on the gap side, which is where a stem pointing into the gap would cross them
    vector<tie_span> spans;
    for (int c = 0; c < chord_count; ++c) {
      const stem_group& g = dm.s_chordData[c].groups[part];
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
        count += in_part(notes[i], part);
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
      spans.push_back({c, end, static_cast<float>(findStaveY(n->sheet_y, stave))});
    }
    if (spans.empty()) {
      continue;
    }

    // a chord the tie passes under (or over) crosses it when its stemmed notes sit on the tied note's side
    auto crosses = [&](int ch) {
      const stem_group& g = dm.s_chordData[ch].groups[part];
      const float near = findStaveY(treble ? g.stem_lo : g.stem_hi, stave);
      for (const auto& span : spans) {
        if (span.start < ch && ch < span.end && (treble ? near <= span.y : near >= span.y)) {
          return true;
        }
      }
      return false;
    };
    // a beam may overshoot by as much as its stems can shorten, since it is pulled back inside the sheet when drawn
    auto fits_away = [&](int first, int last, int lo, int hi, float len, float give) {
      return treble ? findStaveY(hi, stave) - len + give >= find_stem_edge(dm, first, last, part, true)
                    : findStaveY(lo, stave) + len - give <= find_stem_edge(dm, first, last, part, false);
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
        const stem_group& g = dm.s_chordData[ch].groups[part];
        lo = std::min(lo, g.stem_lo);
        hi = std::max(hi, g.stem_hi);
        levels = std::max(levels, g.flag_type - FLAGTYPE_8);
      }
      if (std::ranges::any_of(b.chords, crosses) &&
          fits_away(b.chords.front(), b.chords.back(), lo, hi, beam_stem_len + levels * beam_step + beam_thickness,
                    beam_stem_len - beam_min_len)) {
        b.dir = away;
        b.limited = false;
        for (int ch : b.chords) {
          dm.s_chordData[ch].groups[part].dir = away;
        }
      }
    }

    for (int ch = 0; ch < chord_count; ++ch) {
      stem_group& g = dm.s_chordData[ch].groups[part];
      if (g.empty() || !g.has_stem() || g.beam >= 0 || g.dir != toward_gap) {
        continue;
      }
      const int extra = g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0;
      if (crosses(ch) && fits_away(ch, ch, g.stem_lo, g.stem_hi, stem_len + extra, 0)) {
        g.dir = away;
      }
    }
  }
}

void sheetController::find_chord_widths(sheetMeasure& dm) {
  const int head = getSymbolWidth(SYM_HEAD_STD);

  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    sheetChordData& cd = dm.s_chordData[ch];
    const auto& c = dm.chords[ch].second;

    int left = 0;
    int right = 0;

    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& upper = cd.groups[find_part(stave, 0)];
      stem_group& lower = cd.groups[find_part(stave, 1)];
      if (upper.empty() && lower.empty()) {
        continue;
      }

      for (int part = find_part(stave, 0); part <= find_part(stave, 1); ++part) {
        stem_group& g = cd.groups[part];
        g.head_width = 0;
        for (int i = g.first; !g.empty() && i <= g.last; ++i) {
          if (in_part(c[i], part)) {
            g.head_width = std::max(g.head_width, getSymbolWidth(getSymbolType(c[i]->display_type)));
          }
        }
      }

      // a lower voice reaching the upper one steps right, so their stems line up between the heads
      lower.shift = 0;
      if (!upper.empty() && !lower.empty() && upper.lo - lower.hi <= 1) {
        const int upper_right = upper.flipped && upper.dir != FLAG_DOWN ? head + upper.head_width : upper.head_width;
        const int lower_left = lower.flipped && lower.dir == FLAG_DOWN ? -head : 0;
        lower.shift = upper_right - lower_left;
        // crossing voices would have their stems side by side over the same notes, so they move further apart
        if (upper.has_stem() && lower.has_stem() && upper.stem_lo < lower.stem_hi) {
          lower.shift += voice_cross_gap;
        }
      }

      // accidentals of both voices stack into columns leftwards when they would overlap vertically
      int acc_cols = 0;
      int col_y[acc_col_max] = {};
      for (int i = static_cast<int>(c.size()) - 1; i >= 0; --i) {
        sheetNote* n = c[i];
        if (!n->visible || n->out_of_bounds || n->stave != stave || !has_acc(n)) {
          continue;
        }

        const int y = n->sheet_y;
        int col = 0;
        while (col < acc_cols && col_y[col] - y < acc_clearance) {
          ++col;
        }
        col = std::min(col, acc_col_max - 1);
        acc_cols = std::max(acc_cols, col + 1);
        col_y[col] = y;
        n->acc_col = col;
      }

      int stave_left = 0;
      for (const stem_group* g : {&upper, &lower}) {
        if (!g->empty() && g->flipped && g->dir == FLAG_DOWN) {
          stave_left = std::max(stave_left, head - g->shift);
        }
      }
      if (acc_cols) {
        left = std::max(left, stave_left + accSpacing + acc_cols * acc_col_width);
      }
      left = std::max(left, stave_left);

      for (int part = find_part(stave, 0); part <= find_part(stave, 1); ++part) {
        stem_group& g = cd.groups[part];
        if (g.empty()) {
          continue;
        }
        // measured from the voice's own anchor, so both voices share the accidental columns
        g.acc_x = stave_left + accSpacing + g.shift;

        int g_right = g.head_width;
        if (g.flipped && g.dir != FLAG_DOWN) {
          g_right = head + g.head_width;
        }

        // dots sit right after the noteheads, tucked under an up flag rather than past it
        if (g.dots > 0) {
          g.dot_x = g_right + dotSpacing;
          // the upper voice's dots also step past a lower voice moved in beside them
          if (part_voice(part) == 0 && lower.shift > 0) {
            g.dot_x = std::max(g.dot_x, lower.shift + lower.head_width + dotSpacing);
          }
          g_right = g.dot_x + find_dots_width(g.dots);
        }

        if (g.has_flag()) {
          const int flag_w = getSymbolWidth(g.flag_type, g.dir);
          g_right = std::max(g_right, g.dir == FLAG_DOWN ? flag_w : head - stemWidth + flag_w);
        }

        right = std::max(right, g.shift + g_right);
      }
    }

    // a rolled chord's wavy line stands left of its noteheads and accidentals
    cd.arpeggio_x = 0;
    if (std::ranges::any_of(c, [](const sheetNote* n) { return n->arpeggio >= 0 && !n->out_of_bounds; })) {
      cd.arpeggio_x = std::lround(left + arpeggio_gap + arpeggio_width / 2);
      left = std::lround(left + arpeggio_gap + arpeggio_width);
    }

    // grace notes stand left of everything else, each stave's in a row
    cd.grace_x = 0;
    int grace_w[2] = {0, 0};
    for (const sheetNote* g : dm.graces) {
      if (g->displayBegin == dm.chords[ch].first && !g->out_of_bounds) {
        grace_w[g->stave] += find_grace_width(g);
      }
    }
    if (grace_w[STAVE_TREBLE] + grace_w[STAVE_BASS] > 0) {
      cd.grace_x = left + grace_lead + std::max(grace_w[STAVE_TREBLE], grace_w[STAVE_BASS]);
      left = cd.grace_x;
    }

    // rests are centered on the notehead column
    for (int part = 0; part < part_count; ++part) {
      if (cd.rests[part] < 0) {
        continue;
      }
      const int rest_w = getSymbolWidth(get_rest_symbol(cd.rests[part]));
      const int rest_x = (head - rest_w) / 2;
      const int dot_w = is_dotted_type(cd.rests[part]) ? dotSpacing + getSymbolWidth(SYM_DOT) : 0;
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
  octave_spans.clear();
  add_obstacle(clef_x, clef_x + clef_width, findStaveY(4, STAVE_TREBLE) - clef_height, findStaveY(10, STAVE_TREBLE));

  int tick_delta = std::numeric_limits<int>::min();
  int tick_offset = offset;
  int barline_x = offset;
  int open_octave[2] = {-1, -1};
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
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      add_octave_span(dm, stave, open_octave[stave], offset);
    }

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
    draw_tie(t.x, x1, t.y, t.up, t.flip_y, t.head_note, t.note, t.top, t.bottom);
  }

  // so do octave lines, which then have no hook
  const int next = measureRange.second;
  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    if (open_octave[stave] >= 0 && next < static_cast<int>(displayMeasure.size()) &&
        starts_moved(displayMeasure[next], stave)) {
      octave_spans[open_octave[stave]].x1 = std::max(octave_spans[open_octave[stave]].x1, system_end);
      octave_spans[open_octave[stave]].open_end = true;
    }
  }
  for (const auto& span : octave_spans) {
    draw_octave_line(span);
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

bool sheetController::is_moved(const sheetNote* n) const {
  return n->sheet_y != ctr.getStream().notes[n->oriNote].sheetY;
}

bool sheetController::starts_moved(const sheetMeasure& dm, int stave) const {
  // whether the first notes of the stave sit under a line
  for (const auto& c : dm.chords) {
    for (const sheetNote* n : c.second) {
      if (n->stave == stave) {
        return is_moved(n);
      }
    }
  }
  return false;
}

void sheetController::add_octave_span(const sheetMeasure& dm, int stave, int& open, int measure_end) {
  // a line runs over the chords whose notes on the stave are moved, until those notes stop sounding, and carries on
  // the line of the measure before when there is one
  const bool treble = stave == STAVE_TREBLE;
  const int count = dm.chords.size();
  for (int ch = 0; ch < count; ++ch) {
    bool held = false;
    bool moved = false;
    int until = dm.chords[ch].first;
    for (const sheetNote* n : dm.chords[ch].second) {
      if (n->stave == stave) {
        held = true;
        moved = is_moved(n);
        until = std::max(until, dm.chords[ch].first + note_len(n));
      }
    }
    if (!held) {
      continue;
    }
    if (!moved) {
      open = -1;
      continue;
    }

    float x1 = measure_end;
    for (int e = ch + 1; e < count; ++e) {
      auto back_in = [&](const sheetNote* n) { return n->stave == stave && !is_moved(n); };
      const bool back = std::ranges::any_of(dm.chords[e].second, back_in);
      if (dm.chords[e].first >= until || back) {
        x1 = chord_x[e] - dm.s_chordData[e].leftWidth - octave_pad;
        break;
      }
    }
    x1 = std::max<float>(x1, chord_x[ch] + dm.s_chordData[ch].rightWidth);
    const float edge = treble ? chord_top[ch] : chord_bottom[ch];
    float reach = treble ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
    for (int k = 0; k < count; ++k) {
      if (std::abs(chord_x[k] + head_width_exact / 2 - x1) < octave_hook_window) {
        reach = treble ? std::min(reach, chord_top[k]) : std::max(reach, chord_bottom[k]);
      }
    }
    if (open >= 0) {
      octave_span& span = octave_spans[open];
      span.x1 = x1;
      span.edge = treble ? std::min(span.edge, edge) : std::max(span.edge, edge);
      span.reach = reach;
      continue;
    }
    const float x0 = chord_x[ch] - dm.s_chordData[ch].leftWidth;
    octave_spans.push_back({stave, x0, x1, edge, reach, false});
    open = octave_spans.size() - 1;
  }
  if (!dm.octave_out[stave]) {
    open = -1;
  }
}

void sheetController::draw_octave_line(const octave_span& span) {
  // the text starts the line at the first note, and a dashed line runs on to a hook over the last
  const bool treble = span.stave == STAVE_TREBLE;
  const int sym = treble ? SYM_OTTAVA_ALTA : SYM_OTTAVA_BASSA;
  const float text_w = getGlyphWidth(sym, octave_size);
  const float half = octave_text_height / 2;

  // the line keeps its usual height unless the notes under it reach further, but stays on the sheet
  float y = 0;
  if (treble) {
    y = std::min<float>(findStaveY(octave_line_y, STAVE_TREBLE), span.edge - octave_pad - half);
    y = std::max(y, find_sheet_top() + half);
  }
  else {
    y = std::max<float>(findStaveY(-octave_line_y, STAVE_BASS), span.edge + octave_pad + half);
    y = std::min(y, find_sheet_bottom() - half);
  }

  const float baseline = glyph_baseline * static_cast<float>(octave_size) / fSize;
  drawSymbol(sym, octave_size, span.x0, y + half - baseline, ctr.bgSheetNote);

  const float x1 = std::max(span.x1, span.x0 + text_w + octave_dash);
  for (float x = span.x0 + text_w + octave_dash_gap; x < x1; x += octave_dash + octave_dash_gap) {
    drawLineEx(x, y, std::min(x + octave_dash, x1), y, 1, ctr.bgSheetNote);
  }
  // the hook stops short of whatever stands under the end of the line, and is left out when nothing fits
  const float hook = std::min(octave_hook, (treble ? span.reach - y : y - span.reach) - 2);
  const bool hooked = !span.open_end && hook >= 2;
  if (hooked) {
    drawLineEx(x1, y, x1, y + (treble ? hook : -hook), 1, ctr.bgSheetNote);
  }

  if (treble) {
    add_obstacle(span.x0, x1, y - half, y + (hooked ? hook : 1));
  }
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
    for (int part = 0; part < part_count; ++part) {
      draw_group_shapes(dm, ch, part, measure_x);
    }
    draw_graces(dm, ch, false);
    draw_oob_markers(dm, ch);
  }
  for (unsigned int b = 0; b < dm.beams.size(); ++b) {
    draw_beam(dm, b);
  }

  const int head = getSymbolWidth(SYM_HEAD_STD);
  for (unsigned int ch = 0; ch < dm.chords.size(); ++ch) {
    for (int part = 0; part < part_count; ++part) {
      const int rest = dm.s_chordData[ch].rests[part];
      if (rest >= 0) {
        const int stave = part_stave(part);
        const int shift = dm.s_chordData[ch].rest_shift[part];
        const int rest_w = getSymbolWidth(get_rest_symbol(rest));
        const float rest_x = chord_x[ch] + (head - rest_w) / 2;
        draw_rest(rest, stave, rest_x, shift);
        // an upper voice's rest can reach above the stave
        if (stave == STAVE_TREBLE) {
          const int y = find_rest_y(rest, stave, shift);
          add_obstacle(rest_x, rest_x + rest_w, y - find_rest_extent(rest, false), y + find_rest_extent(rest, true));
        }
      }
      draw_group_glyphs(dm, ch, part);
    }
    draw_graces(dm, ch, true);
    draw_arpeggios(dm, ch);
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

int sheetController::find_rest_y(int type, int stave, int shift) {
  // whole rests hang from the second line, the rest sit on the middle line
  const int line = type == NOTE_WHOLE ? (stave == STAVE_TREBLE ? 8 : -4) : (stave == STAVE_TREBLE ? 6 : -6);
  return findStaveY(line + shift, stave) + (type == NOTE_WHOLE ? 1 : 0);
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

void sheetController::draw_rest(int type, int stave, float x, int shift) {
  const int y = find_rest_y(type, stave, shift);
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

int sheetController::stem_anchor(const sheetMeasure& dm, const beam_group& beam, int ch) const {
  return chord_x[ch] + dm.s_chordData[ch].groups[beam.part()].shift;
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

int sheetController::find_dot_x(const sheetMeasure& dm, int ch, int part, const sheetNote* n) {
  // a dot follows whatever is on its own row, so it only moves past a flipped notehead or a lower voice right beside it
  const auto& c = dm.chords[ch].second;
  const stem_group& g = dm.s_chordData[ch].groups[part];
  const int row = find_dot_row(dm, part, n->sheet_y);

  auto near_row = [&](const sheetNote* m) { return std::abs(m->sheet_y - row) <= 1; };

  int right = g.head_width;
  if (g.flipped && g.dir != FLAG_DOWN) {
    for (int i = g.first; i <= g.last; ++i) {
      if (in_part(c[i], part) && !c[i]->left && near_row(c[i])) {
        right = getSymbolWidth(SYM_HEAD_STD) + g.head_width;
      }
    }
  }
  if (part_voice(part) == 0) {
    const stem_group& lower = dm.s_chordData[ch].groups[part + 1];
    for (int i = lower.first; lower.shift > 0 && i <= lower.last; ++i) {
      if (in_part(c[i], part + 1) && near_row(c[i])) {
        right = std::max(right, lower.shift + lower.head_width);
      }
    }
  }
  return right + dotSpacing;
}

int sheetController::find_dots_width(int dots) {
  return dots * getSymbolWidth(SYM_DOT) + std::max(0, dots - 1) * dot_gap;
}

int sheetController::find_dot_row(const sheetMeasure& dm, int part, int sheet_y) const {
  // dots on a line move into the space above, or below for a lower voice
  if (sheet_y % 2 != 0) {
    return sheet_y;
  }
  return dm.two_voice[part_stave(part)] && part_voice(part) == 1 ? sheet_y - 1 : sheet_y + 1;
}

int sheetController::find_flag_y(const stem_group& g, int stave, float top, float bottom) {
  // stems stay close to their notes, longer only when extra flags need the room
  const int len = stem_len + (g.has_flag() ? flag_extra[g.flag_type - FLAGTYPE_8] : 0);
  const int shortest = len - (stem_len - stem_min_len);
  const int lo_y = findStaveY(g.stem_lo, stave);
  const int hi_y = findStaveY(g.stem_hi, stave);

  if (g.dir == FLAG_DOWN) {
    int tip = lo_y + len;
    // pulled back to clear the other stave, but never past the shortest allowed stem
    if (g.limited) {
      tip = std::max<int>(std::min<float>(tip, g.limit), lo_y + shortest);
    }
    // even when nothing else works, a stem stops at the edge of the sheet, and short of an octave line there as
    // long as it keeps its shortest length
    const float edge = std::max<float>(bottom, lo_y + shortest);
    return std::min<int>(tip, std::min(edge, find_sheet_bottom()));
  }
  int tip = hi_y - len;
  if (g.limited) {
    tip = std::min<int>(std::max<float>(tip, g.limit), hi_y - shortest);
  }
  const float edge = std::min<float>(top, hi_y - shortest);
  return std::max<int>(tip, std::max(edge, find_sheet_top()));
}

void sheetController::draw_group_shapes(const sheetMeasure& dm, int ch, int part, int measure_x) {
  const stem_group& g = dm.s_chordData[ch].groups[part];
  if (g.empty()) {
    return;
  }

  const auto& c = dm.chords[ch].second;
  const int stave = part_stave(part);
  const int anchor = chord_x[ch] + g.shift;
  const bool down = g.dir == FLAG_DOWN;

  float top = findStaveY(g.hi, stave) - head_half;
  float bottom = findStaveY(g.lo, stave) + head_half;

  draw_ledger_lines(dm, ch, part, anchor);

  for (int i = g.first; i <= g.last; ++i) {
    const sheetNote* n = c[i];
    if (!in_part(n, part)) {
      continue;
    }

    const int y = findStaveY(n->sheet_y, stave);
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

    // ties around the heads run from centre to centre, ties between them from edge to edge, never from the dots
    auto find_tie_end = [&](bool outside) { return outside ? head_x + head_w * 0.5f : head_x - 2.0f; };

    if (n->tie_in) {
      auto it = std::ranges::find(open_ties, n->oriNote, &open_tie::note);
      if (it != open_ties.end()) {
        const float tie_end = find_tie_end(it->outside);
        draw_tie(it->x, tie_end, it->y, it->up, it->flip_y, it->head_note, n->oriNote, it->top, it->bottom);
        open_ties.erase(it);
      }
      else {
        // the tie started on an earlier page, so it picks up from the start of the measure
        const float tie_end = find_tie_end(n->tie_outside);
        const float tie_x = std::min<float>(measure_x + 2, tie_end - tie_min_len);
        draw_tie(tie_x, tie_end, tie_y, n->tie_up, flip_y, n->oriNote, n->oriNote, find_tie_edge(n, true),
                 find_tie_edge(n, false));
      }
    }
    if (n->tie_out) {
      const float tie_x = n->tie_outside ? head_x + head_w * 0.5f : head_x + head_w + 2.0f;
      open_ties.push_back({n->oriNote, tie_x, tie_y, n->tie_up, n->tie_outside, flip_y, n->oriNote,
                           find_tie_edge(n, true), find_tie_edge(n, false)});
    }
  }

  // a duplicate hidden behind a note at the same position still carries its tie, so it lands on that note
  for (const sheetNote* hidden : c) {
    if (hidden->visible || hidden->out_of_bounds || hidden->stave != stave || hidden->voice != part_voice(part) ||
        !(hidden->tie_in || hidden->tie_out)) {
      continue;
    }
    const int sheet_y = hidden->sheet_y;
    auto twin =
        std::ranges::find_if(c, [&](const sheetNote* n) { return in_part(n, part) && n->sheet_y == sheet_y; });
    if (twin == c.end()) {
      continue;
    }

    const int head_x = find_head_x(g, *twin, anchor);
    const int head_w = getSymbolWidth(getSymbolType((*twin)->display_type));
    if (hidden->tie_in) {
      auto it = std::ranges::find(open_ties, hidden->oriNote, &open_tie::note);
      if (it != open_ties.end()) {
        draw_tie(it->x, it->outside ? head_x + head_w * 0.5f : head_x - 2.0f, it->y, it->up, it->flip_y,
                 it->head_note, (*twin)->oriNote, it->top, it->bottom);
        open_ties.erase(it);
      }
    }
    if (hidden->tie_out) {
      const int y = findStaveY(sheet_y, stave);
      const int tie_off = (*twin)->tie_outside ? tie_outside_offset : tie_offset;
      const float tie_y = y + ((*twin)->tie_up ? -tie_off : tie_off);
      const float flip_y = y + ((*twin)->tie_up ? tie_off : -tie_off);
      const float tie_x = (*twin)->tie_outside ? head_x + head_w * 0.5f : head_x + head_w + 2.0f;
      open_ties.push_back({hidden->oriNote, tie_x, tie_y, (*twin)->tie_up, (*twin)->tie_outside, flip_y,
                           (*twin)->oriNote, find_tie_edge(*twin, true), find_tie_edge(*twin, false)});
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
      tip = find_flag_y(g, stave, find_stem_edge(dm, ch, ch, part, true), find_stem_edge(dm, ch, ch, part, false));
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

void sheetController::draw_group_glyphs(const sheetMeasure& dm, int ch, int part) {
  const stem_group& g = dm.s_chordData[ch].groups[part];
  if (g.empty()) {
    return;
  }

  const colorRGB& col = ctr.bgSheetNote;
  const auto& c = dm.chords[ch].second;
  const int stave = part_stave(part);
  const int anchor = chord_x[ch] + g.shift;

  for (int i = g.first; i <= g.last; ++i) {
    const sheetNote* n = c[i];
    if (!in_part(n, part)) {
      continue;
    }

    const int y = findStaveY(n->sheet_y, stave);
    drawSymbol(getSymbolType(n->display_type), fSize, find_head_x(g, n, anchor), y - glyph_baseline, col);

    if (has_acc(n)) {
      const int acc_sym = getAccType(n->displayAcc);
      const int acc_x = anchor - g.acc_x - n->acc_col * acc_col_width - getSymbolWidth(acc_sym);
      drawSymbol(acc_sym, fSize, acc_x, y - glyph_baseline, col);
    }

    const int dot_y = y - (find_dot_row(dm, part, n->sheet_y) - n->sheet_y) * ctr.barWidth / 2;
    for (int k = 0; k < n->dots; ++k) {
      const int dot_x = anchor + find_dot_x(dm, ch, part, n) + k * (getSymbolWidth(SYM_DOT) + dot_gap);
      drawSymbol(SYM_DOT, fSize, dot_x, dot_y - glyph_baseline, col);
    }
  }

}

void sheetController::draw_flags(float x, float tip, int count, bool down, float scale) {
  // the font's flags have hairline tails that break up at this size, so a thickened trace is drawn instead
  const float scale_y = (count == 1 ? flag_scale_single : flag_scale_stacked) * scale;
  const float dir = down ? -1.0f : 1.0f;

  for (int k = 0; k < count; ++k) {
    const float y0 = tip + dir * k * flag_step * scale;
    Vector2 outer[flag_trace_len];
    Vector2 inner[flag_trace_len];
    for (int i = 0; i < flag_trace_len; ++i) {
      outer[i] = {x + flag_outer[i][0] * scale, y0 + dir * flag_outer[i][1] * scale_y};
      inner[i] = {x + flag_inner[i][0] * scale, y0 + dir * flag_inner[i][1] * scale_y};
    }
    draw_aa_strip(outer, inner, flag_trace_len, ctr.bgSheetNote);
  }
}

void sheetController::draw_arpeggios(const sheetMeasure& dm, int ch) {
  // each rolled chord gets one wavy line from its lowest note to its highest, across both staves when it spans them
  const sheetChordData& cd = dm.s_chordData[ch];
  if (cd.arpeggio_x == 0) {
    return;
  }
  const auto& c = dm.chords[ch].second;
  const float x = chord_x[ch] - cd.arpeggio_x;
  for (const sheetNote* first : c) {
    if (first->arpeggio < 0 || first->out_of_bounds ||
        std::ranges::find(c, first->arpeggio, &sheetNote::arpeggio) != std::ranges::find(c, first)) {
      continue;
    }
    float top = std::numeric_limits<float>::max();
    float bottom = std::numeric_limits<float>::lowest();
    for (const sheetNote* n : c) {
      if (n->arpeggio == first->arpeggio && !n->out_of_bounds) {
        const float y = findStaveY(n->sheet_y, n->stave);
        top = std::min(top, y - arpeggio_ext);
        bottom = std::max(bottom, y + arpeggio_ext);
      }
    }

    // whole segments of leland's wiggle turned upright, centred on the notes, each glyph's baseline running up its
    // right edge once turned
    const int count = std::max(1, static_cast<int>(std::lround((bottom - top) / arpeggio_segment)));
    const float start = std::round((top + bottom + count * arpeggio_segment) / 2);
    const float right = std::round(x + arpeggio_width / 2);
    for (int i = 0; i < count; ++i) {
      const float y = std::round(start - i * arpeggio_segment);
      drawSymbolRotated(SYM_ARPEGGIO, fSize, right - glyph_baseline, y, -90.0f, ctr.bgSheetNote);
    }
    add_obstacle(x - arpeggio_width / 2, x + arpeggio_width / 2, start - count * arpeggio_segment, start);
  }
}

int sheetController::find_grace_width(const sheetNote* g) {
  const int acc = has_acc(g) ? getGlyphWidth(getAccType(g->displayAcc), grace_size) + grace_acc_gap : 0;
  return acc + getGlyphWidth(SYM_HEAD_STD, grace_size) + grace_gap;
}

void sheetController::draw_graces(const sheetMeasure& dm, int ch, bool glyphs) {
  const sheetChordData& cd = dm.s_chordData[ch];
  if (cd.grace_x == 0) {
    return;
  }
  const float scale = static_cast<float>(grace_size) / fSize;
  const float baseline = glyph_baseline * scale;
  const int head_w = getGlyphWidth(SYM_HEAD_STD, grace_size);
  const float stroke = stemWidth * scale + 0.4f;

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    // each stave's grace notes in a row ending before the chord, with stems up unless they belong to a lower voice
    struct grace_head {
      const sheetNote* n;
      float x;
      float y;
    };
    grace_head row[2 * grace_max_count];
    int count = 0;
    bool down = false;
    float x = chord_x[ch] - cd.grace_x;
    for (const sheetNote* g : dm.graces) {
      if (g->displayBegin != dm.chords[ch].first || g->out_of_bounds || g->stave != stave ||
          count == 2 * grace_max_count) {
        continue;
      }
      const float acc = has_acc(g) ? getGlyphWidth(getAccType(g->displayAcc), grace_size) + grace_acc_gap : 0;
      row[count++] = {g, x + acc, static_cast<float>(findStaveY(g->sheet_y, stave))};
      down = down || (dm.two_voice[stave] && g->voice == 1);
      x += find_grace_width(g);
    }
    if (count == 0) {
      continue;
    }

    if (glyphs) {
      for (int i = 0; i < count; ++i) {
        const sheetNote* g = row[i].n;
        drawSymbol(SYM_HEAD_STD, grace_size, row[i].x, row[i].y - baseline, ctr.bgSheetNote);
        if (has_acc(g)) {
          const int acc = getAccType(g->displayAcc);
          drawSymbol(acc, grace_size, row[i].x - grace_acc_gap - getGlyphWidth(acc, grace_size), row[i].y - baseline,
                     ctr.bgSheetNote);
        }
      }
      continue;
    }

    // short ledger lines for heads off the stave
    const int top_line = stave == STAVE_TREBLE ? 10 : -2;
    const int bottom_line = stave == STAVE_TREBLE ? 2 : -10;
    for (int i = 0; i < count; ++i) {
      const int sheet_y = row[i].n->sheet_y;
      for (int line = top_line + 2; line <= sheet_y; line += 2) {
        drawLineEx(row[i].x - 2, findStaveY(line, stave), row[i].x + head_w + 2, findStaveY(line, stave), 1,
                   ctr.bgSheetNote);
      }
      for (int line = bottom_line - 2; line >= sheet_y; line -= 2) {
        drawLineEx(row[i].x - 2, findStaveY(line, stave), row[i].x + head_w + 2, findStaveY(line, stave), 1,
                   ctr.bgSheetNote);
      }
    }

    // stems reach a line through the first and last tips, moved out until every stem is long enough
    const float sign = down ? 1.0f : -1.0f;
    auto stem_x = [&](int i) { return down ? row[i].x + stroke / 2 : row[i].x + head_w - stroke / 2; };
    const float x0 = stem_x(0);
    const float dx = stem_x(count - 1) - x0;
    const float slope =
        dx > 0 ? std::clamp((row[count - 1].y - row[0].y) / dx, -grace_beam_slope, grace_beam_slope) : 0;
    float y0 = row[0].y + sign * grace_stem_len;
    for (int i = 0; i < count; ++i) {
      const float tip = row[i].y + sign * grace_stem_len - slope * (stem_x(i) - x0);
      y0 = down ? std::max(y0, tip) : std::min(y0, tip);
    }
    auto tip_y = [&](int i) { return y0 + slope * (stem_x(i) - x0); };

    float top = std::numeric_limits<float>::max();
    float bottom = std::numeric_limits<float>::lowest();
    for (int i = 0; i < count; ++i) {
      drawLineEx(stem_x(i), row[i].y + sign, stem_x(i), tip_y(i), stroke, ctr.bgSheetNote);
      top = std::min({top, row[i].y - head_half, tip_y(i)});
      bottom = std::max({bottom, row[i].y + head_half, tip_y(i)});
    }

    if (count == 1) {
      // a lone grace note is flagged and struck through
      const float tip = tip_y(0);
      draw_flags(stem_x(0) - stroke / 2, tip, 1, down, scale);
      const Vector2 a[2] = {{stem_x(0) - 4.0f, tip - sign * 9.0f}, {stem_x(0) + 6.0f, tip - sign * 2.0f}};
      const Vector2 b[2] = {{stem_x(0) - 4.0f, tip - sign * 9.0f + 1.1f}, {stem_x(0) + 6.0f, tip - sign * 2.0f + 1.1f}};
      draw_aa_strip(a, b, 2, ctr.bgSheetNote);
    }
    else {
      // a group shares beams, one per level its values call for
      const int levels = row[0].n->display_type == NOTE_16 ? 2 : 1;
      for (int level = 0; level < levels; ++level) {
        const float shift = -sign * level * grace_beam_step;
        const float xa = stem_x(0) - stroke / 2;
        const float xb = stem_x(count - 1) + stroke / 2;
        const float ya = y0 + slope * (xa - x0) + shift;
        const float yb = y0 + slope * (xb - x0) + shift;
        const Vector2 a[2] = {{xa, ya}, {xb, yb}};
        const Vector2 b[2] = {{xa, ya - sign * grace_beam_thickness}, {xb, yb - sign * grace_beam_thickness}};
        draw_aa_strip(a, b, 2, ctr.bgSheetNote);
      }
    }

    const float left = row[0].x - (has_acc(row[0].n) ? grace_acc_gap + 8 : 0);
    if (stave == STAVE_TREBLE) {
      chord_top[ch] = std::min(chord_top[ch], top);
      add_obstacle(left, x, top, bottom);
    }
    else {
      chord_bottom[ch] = std::max(chord_bottom[ch], bottom);
    }
  }
}

void sheetController::draw_ledger_lines(const sheetMeasure& dm, int ch, int part, int anchor) {
  const auto& c = dm.chords[ch].second;
  const stem_group& g = dm.s_chordData[ch].groups[part];
  const int stave = part_stave(part);
  const int top_line = stave == STAVE_TREBLE ? 10 : -2;
  const int bottom_line = stave == STAVE_TREBLE ? 2 : -10;

  // each ledger line only reaches across the noteheads at or beyond it
  auto draw_ledger = [&](int line, bool above) {
    float x0 = std::numeric_limits<float>::max();
    float x1 = std::numeric_limits<float>::lowest();
    for (int i = g.first; i <= g.last; ++i) {
      const sheetNote* n = c[i];
      if (!in_part(n, part)) {
        continue;
      }
      const int sheet_y = n->sheet_y;
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
    const stem_group& g = dm.s_chordData[ch].groups[beam.part()];
    return findStaveY(up ? g.stem_hi : g.stem_lo, beam.stave);
  };

  const int first = beam.chords.front();
  const int last = beam.chords.back();
  const float x0 = find_stem_x(stem_anchor(dm, beam, first), beam.dir);
  const float dx = find_stem_x(stem_anchor(dm, beam, last), beam.dir) - x0;

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
    levels = std::max(levels, dm.s_chordData[ch].groups[beam.part()].flag_type - FLAGTYPE_8 + 1);
  }
  const float len = beam_stem_len + (levels - 1) * beam_step;

  // shift the beam until every stem is long enough
  float y0 = up ? std::numeric_limits<float>::max() : std::numeric_limits<float>::lowest();
  for (int ch : beam.chords) {
    const float x = find_stem_x(stem_anchor(dm, beam, ch), beam.dir) - x0;
    float tip = up ? near_y(ch) - len : near_y(ch) + len;

    // the beam also clears a stemless note sitting on its side of the chord
    const stem_group& g = dm.s_chordData[ch].groups[beam.part()];
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
      const float line_y = y0 + slope * (find_stem_x(stem_anchor(dm, beam, ch), beam.dir) - x0);
      shift = std::max(shift, up ? beam.limit - line_y : line_y - beam.limit);
      room = std::min(room, up ? near_y(ch) - min_len - line_y : line_y - near_y(ch) - min_len);

      // nor past a stemless note on the beam's side, which matters more than a little room from the other stave
      const stem_group& g = dm.s_chordData[ch].groups[beam.part()];
      const float clear = head_half + 2 + (levels - 1) * beam_step + beam_thickness;
      room = std::min(room, up ? findStaveY(g.hi, beam.stave) - clear - line_y
                               : line_y - findStaveY(g.lo, beam.stave) - clear);
    }
    y0 += (up ? 1.0f : -1.0f) * std::clamp(shift, 0.0f, std::max(room, 0.0f));
  }

  // even when nothing else works, a beam stays inside the sheet (its other levels stack towards the notes), and short
  // of an octave line there as long as its stems keep their shortest length
  const float min_len = len - (beam_stem_len - beam_min_len);
  float top = find_stem_edge(dm, first, last, beam.part(), true);
  float bottom = find_stem_edge(dm, first, last, beam.part(), false);
  for (int ch : beam.chords) {
    top = up ? std::min(top, near_y(ch) - min_len) : top;
    bottom = up ? bottom : std::max(bottom, near_y(ch) + min_len);
  }
  top = std::max(top, find_sheet_top());
  bottom = std::min(bottom, find_sheet_bottom());
  for (int ch : beam.chords) {
    const float line_y = y0 + slope * (find_stem_x(stem_anchor(dm, beam, ch), beam.dir) - x0);
    if (up && line_y < top) {
      y0 += top - line_y;
    }
    else if (!up && line_y > bottom) {
      y0 -= line_y - bottom;
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

  auto stem_x = [&](int i) { return find_stem_x(stem_anchor(dm, beam, beam.chords[i]), beam.dir); };
  auto levels = [&](int i) { return dm.s_chordData[beam.chords[i]].groups[beam.part()].flag_type - FLAGTYPE_8 + 1; };

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

float sheetController::find_tie_edge(const sheetNote* n, bool top) const {
  // a tie under an octave line keeps clear of the line where it runs along the sheet's edge
  const bool room = is_moved(n) && (n->stave == STAVE_TREBLE) == top;
  if (top) {
    return room ? find_sheet_top() + octave_tie_room : std::numeric_limits<float>::lowest();
  }
  return room ? find_sheet_bottom() - octave_tie_room : std::numeric_limits<float>::max();
}

void sheetController::draw_tie(float x0, float x1, float y, bool up, float flip_y, int start_note, int end_note,
                               float top, float bottom) {
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
  const float thickness = std::clamp(1.1f + len * 0.002f, 1.1f, 1.5f);

  // a tie may cross stems but tries not to cross noteheads, first by reshaping and then by changing sides
  struct candidate {
    float y;
    bool up;
    float height;
  };
  // a flat tie slips between close noteheads, and when nothing fits the usual shape is kept rather than a deep arc
  const float flatter = std::max(tie_min_height, height * 0.5f);
  const candidate candidates[] = {
      {y, up, height},         {y, up, flatter},           {y, up, tie_min_height},
      {y, up, height * 1.6f},  {flip_y, !up, height},      {flip_y, !up, flatter},
      {flip_y, !up, tie_min_height},
  };
  // the first that misses noteheads and stays clear of an octave line, else the first missing noteheads
  auto clear_of_heads = [&](const candidate& c) {
    return !tie_hits_heads(x0, x1, c.y, c.up, c.height, shoulder, std::min(thickness, c.height * 0.7f), start_note,
                           end_note);
  };
  auto clear_of_edge = [&](const candidate& c) {
    return c.up ? c.y - c.height - tie_end_width >= top : c.y + c.height + tie_end_width <= bottom;
  };
  auto clear_of_both = [&](const candidate& c) { return clear_of_heads(c) && clear_of_edge(c); };
  auto it = std::ranges::find_if(candidates, clear_of_both);
  if (it == std::end(candidates)) {
    it = std::ranges::find_if(candidates, clear_of_heads);
  }
  const candidate chosen = it == std::end(candidates) ? candidates[0] : *it;

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
