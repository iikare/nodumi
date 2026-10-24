#include "sheetcomp.h"

#include <algorithm>
#include <climits>
#include <cstdlib>

#include "define.h"
#include "note.h"
#include "sheetctr.h"

using std::max;
using std::min;

namespace {

bool is_drawable(const sheetNote* n, int stave) { return n->visible && !n->out_of_bounds && n->stave == stave; }

int find_stem_dir(int lo, int hi, int stave) {
  // the note furthest from the middle line decides, ties point down
  const int mid = stave == STAVE_TREBLE ? 6 : -6;
  return hi - mid >= mid - lo ? FLAG_DOWN : FLAG_UP;
}

// splits a silence into rests that each start on a multiple of their own length
template <typename F>
void split_rest(int begin, int end, bool compound, F&& emit) {
  constexpr int dotted_beat = 24;
  int pos = begin;
  while (pos < end) {
    const int remain = end - pos;

    // a whole beat of silence in compound time takes one dotted rest
    if (compound && pos % dotted_beat == 0 && remain >= dotted_beat) {
      emit(pos, NOTE_QUARTER_DOT);
      pos += dotted_beat;
      continue;
    }

    const int room = compound ? min(remain, dotted_beat - pos % dotted_beat) : remain;
    int value = note_units[NOTE_WHOLE];
    while (value > 1 && (value > room || pos % value != 0)) {
      value /= 2;
    }
    emit(pos, find_exact_type(value));
    pos += value;
  }
}

}  // namespace

void sheetMeasure::buildChordMap(deque<sheetNote>& vecNote) {
  const midi& stream = ctr.getStream();

  vector<sheetNote*> order;
  order.reserve(vecNote.size());
  for (auto& n : vecNote) {
    order.push_back(&n);
  }

  // chords in time order, notes low to high within each chord
  std::sort(order.begin(), order.end(), [&](const sheetNote* a, const sheetNote* b) {
    if (a->displayBegin != b->displayBegin) {
      return a->displayBegin < b->displayBegin;
    }
    const note& na = stream.notes[a->oriNote];
    const note& nb = stream.notes[b->oriNote];
    if (na.sheetY != nb.sheetY) {
      return na.sheetY < nb.sheetY;
    }
    if (na.y != nb.y) {
      return na.y < nb.y;
    }
    // a continuing tie comes first among duplicates so it is the one left visible
    return a->tie_in > b->tie_in;
  });

  chords.clear();
  for (auto* n : order) {
    if (chords.empty() || chords.back().first != n->displayBegin) {
      chords.push_back(make_pair(n->displayBegin, vector<sheetNote*>{}));
    }
    chords.back().second.push_back(n);
  }

  s_chordData.assign(chords.size(), sheetChordData{});
}

void sheetMeasure::find_rests(int len, bool open) {
  const timeSig& ts = measure->currentTime;
  const bool compound = ts.getBottom() == 8 && ts.getTop() % 3 == 0;

  vector<pair<int, int>> found[2];  // position and value of each rest

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    int cursor = 0;
    bool sounding = false;

    auto fill = [&](int begin, int end) {
      if (end - begin >= rest_min_units) {
        split_rest(begin, end, compound, [&](int pos, int type) { found[stave].push_back(make_pair(pos, type)); });
      }
    };

    // hidden and out-of-bounds notes still sound, so they keep rests away too
    for (const auto& c : chords) {
      for (const sheetNote* n : c.second) {
        if (n->stave != stave) {
          continue;
        }
        sounding = true;
        fill(cursor, c.first);
        cursor = max(cursor, c.first + note_units[n->display_type]);
      }
    }

    // a live measure still being played has no trailing silence yet
    if (!sounding) {
      bar_rest[stave] = !open;
    }
    else if (!open) {
      fill(cursor, len);
    }
  }

  // a rest starting where no note does gets a column of its own
  for (const auto& stave_rests : found) {
    for (const auto& r : stave_rests) {
      auto it = std::lower_bound(chords.begin(), chords.end(), r.first,
                                 [](const auto& c, int pos) { return c.first < pos; });
      if (it == chords.end() || it->first != r.first) {
        chords.insert(it, make_pair(r.first, vector<sheetNote*>{}));
      }
    }
  }

  s_chordData.assign(chords.size(), sheetChordData{});
  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    for (const auto& r : found[stave]) {
      auto it = std::lower_bound(chords.begin(), chords.end(), r.first,
                                 [](const auto& c, int pos) { return c.first < pos; });
      s_chordData[it - chords.begin()].rests[stave] = r.second;
    }
  }
}

void sheetMeasure::build_stem_groups() {
  const midi& stream = ctr.getStream();

  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    const auto& c = chords[ch].second;
    for (int i = 0; i < static_cast<int>(c.size()); ++i) {
      const sheetNote* n = c[i];
      if (!n->visible || n->out_of_bounds || n->stave == STAVE_NONE) {
        continue;
      }

      stem_group& g = s_chordData[ch].groups[n->stave];
      const int sheet_y = stream.notes[n->oriNote].sheetY;
      if (g.empty()) {
        g.first = i;
        g.lo = sheet_y;
        g.hi = sheet_y;
      }
      g.last = i;
      g.hi = max(g.hi, sheet_y);
      // whole notes sit beside the stem rather than on it
      if (n->display_type >= NOTE_HALF_DOT) {
        g.stem_lo = min(g.stem_lo, sheet_y);
        g.stem_hi = max(g.stem_hi, sheet_y);
      }
      // enum is inverted
      g.type = max(g.type, n->display_type);
      g.dotted |= is_dotted_type(n->display_type);
    }

    for (auto& g : s_chordData[ch].groups) {
      if (!g.empty()) {
        g.flag_type = getFlagType(g.type);
      }
    }
  }
}

bool sheetMeasure::is_plain_half_bar(int stave, int half) const {
  // four eighths filling half of the bar may share one beam
  int count = 0;
  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    const int pos = chords[ch].first;
    if (pos < half * 32 || pos >= half * 32 + 32 || s_chordData[ch].groups[stave].empty()) {
      continue;
    }
    if (s_chordData[ch].groups[stave].type != NOTE_8 || pos != half * 32 + count * 8) {
      return false;
    }
    ++count;
  }
  return count == 4;
}

void sheetMeasure::build_beams() {
  beams.clear();

  const timeSig& ts = measure->currentTime;
  const bool compound = ts.getBottom() == 8 && ts.getTop() % 3 == 0;
  const bool common = (ts.getTop() == 4 && ts.getBottom() == 4) || (ts.getTop() == 2 && ts.getBottom() == 2);

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    const bool half_bar[2] = {common && is_plain_half_bar(stave, 0), common && is_plain_half_bar(stave, 1)};

    // beams never cross a beat (or half bar, or dotted beat in compound meter)
    auto find_window = [&](int pos) {
      if (compound) {
        return pos / 24;
      }
      if (pos < 64 && half_bar[pos / 32]) {
        return 100 + pos / 32;
      }
      return pos / 16;
    };

    vector<int> run;
    int run_end = 0;
    int run_window = -1;

    auto flush = [&]() {
      if (run.size() > 1) {
        for (int ch : run) {
          s_chordData[ch].groups[stave].beam = beams.size();
        }
        beams.push_back({stave, FLAG_UP, run, false, 0});
      }
      run.clear();
    };

    for (unsigned int ch = 0; ch < chords.size(); ++ch) {
      const stem_group& g = s_chordData[ch].groups[stave];
      if (g.empty()) {
        continue;
      }
      if (g.flag_type < FLAGTYPE_8 || g.flag_type > FLAGTYPE_64) {
        flush();
        continue;
      }

      const int pos = chords[ch].first;
      const int window = find_window(pos);

      // a rest or a beat boundary breaks the beam
      if (!run.empty() && (window != run_window || pos >= run_end + rest_min_units)) {
        flush();
      }
      if (run.empty()) {
        run_window = window;
      }
      run.push_back(ch);
      run_end = pos + note_units[g.type];
    }
    flush();
  }
}

void sheetMeasure::find_stem_prefs() {
  const midi& stream = ctr.getStream();

  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    const int pos = chords[ch].first;
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& g = s_chordData[ch].groups[stave];
      if (g.empty() || !g.has_stem()) {
        continue;
      }

      bool above = false;
      bool below = false;
      auto check = [&](const sheetNote* n) {
        const int sheet_y = stream.notes[n->oriNote].sheetY;
        above |= sheet_y > g.stem_hi;
        below |= sheet_y < g.stem_lo;
      };

      // notes still sounding from earlier chords on this stave
      for (unsigned int prev = 0; prev < ch; ++prev) {
        for (const sheetNote* n : chords[prev].second) {
          if (is_drawable(n, stave) && chords[prev].first + note_units[n->display_type] > pos) {
            check(n);
          }
        }
      }
      // whole notes starting alongside, which take no stem
      for (int i = g.first; i <= g.last; ++i) {
        const sheetNote* n = chords[ch].second[i];
        if (is_drawable(n, stave) && n->display_type < NOTE_HALF_DOT) {
          check(n);
        }
      }

      if (above != below) {
        g.pref = above ? FLAG_DOWN : FLAG_UP;
      }
    }
  }
}

void sheetMeasure::set_stem_dirs() {
  for (auto& b : beams) {
    int lo = INT_MAX;
    int hi = INT_MIN;
    int prefer_up = 0;
    int prefer_down = 0;
    for (int ch : b.chords) {
      const stem_group& g = s_chordData[ch].groups[b.stave];
      lo = min(lo, g.stem_lo);
      hi = max(hi, g.stem_hi);
      prefer_up += g.pref == FLAG_UP;
      prefer_down += g.pref == FLAG_DOWN;
    }
    // a beam follows the voicing when its members agree, otherwise the pitch rule
    if (prefer_up != prefer_down && (prefer_up == 0 || prefer_down == 0)) {
      b.dir = prefer_up ? FLAG_UP : FLAG_DOWN;
    }
    else {
      b.dir = find_stem_dir(lo, hi, b.stave);
    }
    for (int ch : b.chords) {
      s_chordData[ch].groups[b.stave].dir = b.dir;
    }
  }

  for (auto& cd : s_chordData) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& g = cd.groups[stave];
      if (g.empty() || g.beam >= 0) {
        continue;
      }
      if (!g.has_stem()) {
        g.dir = find_stem_dir(g.lo, g.hi, stave);
      }
      else {
        g.dir = g.pref != FLAG_NONE ? g.pref : find_stem_dir(g.stem_lo, g.stem_hi, stave);
      }
    }
  }
}

void sheetMeasure::place_heads() {
  const midi& stream = ctr.getStream();

  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    auto& c = chords[ch].second;
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      stem_group& g = s_chordData[ch].groups[stave];
      if (g.empty()) {
        continue;
      }

      int count = 0;
      for (int i = g.first; i <= g.last; ++i) {
        count += is_drawable(c[i], stave);
      }

      // walk away from the stem end so the second of each interval of a second flips over
      const bool down = g.dir == FLAG_DOWN;
      const int step = down ? -1 : 1;
      const int stop = down ? g.first - 1 : g.last + 1;
      int prev_y = 0;
      bool prev_normal = false;
      int k = 0;
      for (int i = down ? g.last : g.first; i != stop; i += step) {
        sheetNote* n = c[i];
        if (!is_drawable(n, stave)) {
          continue;
        }

        const int y = stream.notes[n->oriNote].sheetY;
        const bool flip = prev_normal && abs(y - prev_y) == 1;
        n->left = down ? flip : !flip;
        g.flipped |= flip;
        prev_normal = !flip;
        prev_y = y;

        // outer notes tie away from the chord, a lone or middle note ties away from the stem
        const int from_bottom = down ? count - 1 - k : k;
        n->tie_up = 2 * from_bottom == count - 1 ? down : 2 * from_bottom > count - 1;
        // lone notes and the outer notes of a chord tie around the heads, inner ones between them
        n->tie_outside = count == 1 || (from_bottom == count - 1 && n->tie_up) || (from_bottom == 0 && !n->tie_up);
        ++k;
      }
    }
  }
}

int sheetMeasure::getFlagType(const int noteType) {
  switch (noteType) {
    case NOTE_LARGE:
    case NOTE_WHOLE_DOT:
    case NOTE_WHOLE:
      return FLAGTYPE_NONE;
    case NOTE_HALF_DOT:
    case NOTE_HALF:
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
      return FLAGTYPE_STEM;
    case NOTE_8_DOT:
    case NOTE_8:
      return FLAGTYPE_8;
    case NOTE_16_DOT:
    case NOTE_16:
      return FLAGTYPE_16;
    case NOTE_32:
      return FLAGTYPE_32;
    case NOTE_64:
      return FLAGTYPE_64;
    default:
      logQ("invalid note type:", noteType);
      break;
  }

  return FLAGTYPE_NONE;
}
