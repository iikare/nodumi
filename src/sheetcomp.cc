#include "sheetcomp.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstdlib>

#include "define.h"
#include "note.h"
#include "sheetctr.h"

using std::max;
using std::min;

namespace {

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
    // a continuing tie comes first among duplicates so it is the one left visible, then the longest so the rests
    // after it line up
    if (a->tie_in != b->tie_in) {
      return a->tie_in > b->tie_in;
    }
    return a->display_type < b->display_type;
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

void sheetMeasure::find_voices(int len) {
  const midi& stream = ctr.getStream();

  // notes sounding over the same span form one group, and the pieces of a tied note share its span
  struct voice_group {
    int stave;
    int begin;
    int end;
    float pitch;  // mean sheetY
    int lo;
    int hi;
    int count;
    int voice;
  };
  vector<voice_group> groups;
  vector<pair<int, int>> note_group;  // original note and its group

  struct note_span {
    int note;
    int stave;
    int begin;
    int end;
    int sheet_y;
  };
  vector<note_span> spans;
  for (const auto& c : chords) {
    for (sheetNote* n : c.second) {
      if (n->out_of_bounds || n->stave == STAVE_NONE) {
        continue;
      }
      const int end = c.first + note_units[n->display_type];
      auto it = std::find_if(spans.begin(), spans.end(), [&](const note_span& sp) { return sp.note == n->oriNote; });
      if (it == spans.end()) {
        spans.push_back({n->oriNote, n->stave, c.first, end, stream.notes[n->oriNote].sheetY});
      }
      else {
        it->begin = min(it->begin, c.first);
        it->end = max(it->end, end);
      }
    }
  }
  for (const auto& sp : spans) {
    auto it = std::find_if(groups.begin(), groups.end(), [&](const voice_group& g) {
      return g.stave == sp.stave && g.begin == sp.begin && g.end == sp.end;
    });
    if (it == groups.end()) {
      groups.push_back({sp.stave, sp.begin, sp.end, 0, sp.sheet_y, sp.sheet_y, 0, -1});
      it = groups.end() - 1;
    }
    it->pitch += sp.sheet_y;
    it->lo = min(it->lo, sp.sheet_y);
    it->hi = max(it->hi, sp.sheet_y);
    ++it->count;
    note_group.push_back(make_pair(sp.note, it - groups.begin()));
  }
  for (auto& g : groups) {
    g.pitch /= g.count;
  }

  // groups sounding together for long enough cannot share a stem line
  auto conflicts = [](const voice_group& a, const voice_group& b) {
    const int overlap = min(a.end, b.end) - max(a.begin, b.begin);
    const int shorter = min(a.end - a.begin, b.end - b.begin);
    return overlap > max(legato_units, shorter / 2);
  };

  for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
    vector<voice_group*> members;
    for (auto& g : groups) {
      if (g.stave == stave) {
        members.push_back(&g);
      }
    }

    // each group goes above or below the groups it sounds against, by pitch
    vector<voice_group*> solo;
    bool split = false;
    for (voice_group* g : members) {
      int above = 0;
      int below = 0;
      int others = 0;
      float other_pitch = 0;
      float other_len = 0;
      for (const voice_group* h : members) {
        if (h == g || !conflicts(*g, *h)) {
          continue;
        }
        above += g->pitch > h->pitch;
        below += g->pitch < h->pitch;
        other_pitch += h->pitch;
        other_len += h->end - h->begin;
        ++others;
      }
      if (others == 0) {
        solo.push_back(g);
        continue;
      }
      split = true;
      if (above != below) {
        g->voice = above > below ? 0 : 1;
      }
      else if (g->pitch != other_pitch / others) {
        g->voice = g->pitch > other_pitch / others ? 0 : 1;
      }
      else {
        // a unison against a held note, the moving line goes on top
        g->voice = g->end - g->begin <= other_len / others ? 0 : 1;
      }
    }

    int voice_count[2] = {0, 0};
    for (const voice_group* g : members) {
      if (g->voice >= 0) {
        ++voice_count[g->voice];
      }
    }
    split = split && voice_count[0] > 0 && voice_count[1] > 0 && !one_voice[stave];

    // a crowded stave stays in one voice
    vector<pair<int, int>> heads;
    for (const auto& sp : spans) {
      if (sp.stave == stave) {
        heads.push_back(make_pair(sp.begin, sp.sheet_y));
      }
    }
    std::sort(heads.begin(), heads.end());
    heads.erase(std::unique(heads.begin(), heads.end()), heads.end());
    split = split && static_cast<int>(heads.size()) * note_units[NOTE_QUARTER] <= voice_max_density * len;

    // the rest continue whichever line is nearest in time, and the closer one in pitch when both are about as near
    std::sort(solo.begin(), solo.end(), [](const voice_group* a, const voice_group* b) { return a->begin < b->begin; });
    for (voice_group* g : solo) {
      int best_gap[2] = {INT_MAX, INT_MAX};
      float best_dist[2] = {0, 0};
      for (const voice_group* h : members) {
        if (!split || h->voice < 0) {
          continue;
        }
        const int gap = max(0, max(h->begin - g->end, g->begin - h->end));
        const float dist = std::abs(h->pitch - g->pitch);
        if (gap < best_gap[h->voice] || (gap == best_gap[h->voice] && dist < best_dist[h->voice])) {
          best_gap[h->voice] = gap;
          best_dist[h->voice] = dist;
        }
      }
      if (best_gap[0] == INT_MAX && best_gap[1] == INT_MAX) {
        continue;
      }
      if (best_gap[0] == INT_MAX || best_gap[1] == INT_MAX ||
          std::abs(best_gap[0] - best_gap[1]) > legato_units) {
        g->voice = best_gap[0] <= best_gap[1] ? 0 : 1;
      }
      else {
        g->voice = best_dist[0] <= best_dist[1] ? 0 : 1;
      }
    }

    // voices that keep crossing into each other's register are no clearer than one
    int pairs = 0;
    int crossings = 0;
    for (const voice_group* g : members) {
      for (const voice_group* h : members) {
        if (split && g->voice == 0 && h->voice == 1 && min(g->end, h->end) > max(g->begin, h->begin)) {
          ++pairs;
          crossings += h->hi > g->lo;
        }
      }
    }
    split = split && crossings * 3 <= pairs;

    two_voice[stave] = split;
    if (!split) {
      for (voice_group* g : members) {
        g->voice = 0;
      }
      continue;
    }

    float pitch_sum[2] = {0, 0};
    int note_count[2] = {0, 0};
    for (const voice_group* g : members) {
      pitch_sum[g->voice] += g->pitch * g->count;
      note_count[g->voice] += g->count;
    }
    voice_gap[stave] = pitch_sum[0] / note_count[0] - pitch_sum[1] / note_count[1];
  }

  for (const auto& c : chords) {
    for (sheetNote* n : c.second) {
      auto it =
          std::find_if(note_group.begin(), note_group.end(), [&](const auto& ng) { return ng.first == n->oriNote; });
      if (it != note_group.end()) {
        n->voice = groups[it->second].voice;
      }
      else {
        // notes off the stave keep rests away from the voice on their side
        n->voice = n->stave == STAVE_BASS && two_voice[STAVE_BASS] ? 1 : 0;
      }
    }
  }
}

void sheetMeasure::close_gaps(int len, bool open) {
  // a note stopping less than a rest short of the next one in its part is held until it, when that makes one value,
  // so a performed gap leaves no hole in the measure
  for (int part = 0; part < part_count; ++part) {
    const int stave = part_stave(part);
    const int voice = part_voice(part);
    auto in_line = [&](const sheetNote* n) { return n->stave == stave && n->voice == voice; };

    int next = open ? -1 : len;
    for (int ch = static_cast<int>(chords.size()) - 1; ch >= 0; --ch) {
      const int pos = chords[ch].first;
      bool present = false;
      for (sheetNote* n : chords[ch].second) {
        if (!in_line(n)) {
          continue;
        }
        present = true;
        const int end = pos + note_units[n->display_type];
        const int type = next < 0 ? NOTE_NONE : find_exact_type(next - pos);
        if (!n->tie_out && end < next && next - end < rest_min_units && type != NOTE_NONE) {
          n->display_type = type;
        }
      }
      if (present) {
        next = pos;
      }
    }
  }
}

void sheetMeasure::find_rests(int len, bool open) {
  const timeSig& ts = measure->currentTime;
  const bool compound = ts.getBottom() == 8 && ts.getTop() % 3 == 0;

  struct rest_span {
    int pos;
    int type;
    bool edge;  // before the voice's first note or after its last
  };
  vector<rest_span> found[part_count];
  vector<pair<int, int>> sounding[part_count];  // when each part has a note, hidden and out-of-bounds ones included
  int onsets[part_count] = {0, 0, 0, 0};

  for (int part = 0; part < part_count; ++part) {
    const int stave = part_stave(part);
    const int voice = part_voice(part);
    if (voice == 1 && !two_voice[stave]) {
      continue;
    }

    int cursor = 0;

    auto fill = [&](int begin, int end, bool edge) {
      if (end - begin >= rest_min_units) {
        split_rest(begin, end, compound, [&](int pos, int type) { found[part].push_back({pos, type, edge}); });
      }
    };

    // hidden and out-of-bounds notes still sound, so they keep rests away too
    for (const auto& c : chords) {
      bool onset = false;
      for (const sheetNote* n : c.second) {
        if (n->stave != stave || n->voice != voice) {
          continue;
        }
        onset = true;
        fill(cursor, c.first, sounding[part].empty());
        const int end = c.first + note_units[n->display_type];
        sounding[part].push_back(make_pair(c.first, end));
        cursor = max(cursor, end);
      }
      onsets[part] += onset;
    }

    // a live measure still being played has no trailing silence yet
    if (sounding[part].empty()) {
      bar_rest[stave] = !open;
    }
    else if (!open) {
      fill(cursor, len, true);
    }
  }

  // while the other voice plays, a voice leaves out short rests, and the sparser voice also those before it joins
  // or after it leaves
  for (int part = 0; part < part_count; ++part) {
    if (!two_voice[part_stave(part)]) {
      continue;
    }
    const auto& other = sounding[part ^ 1];
    const bool sparser = onsets[part] < onsets[part ^ 1];
    std::erase_if(found[part], [&](const rest_span& r) {
      const int end = r.pos + note_units[r.type];
      return (note_units[r.type] < note_units[NOTE_QUARTER] || (r.edge && sparser)) &&
             std::any_of(other.begin(), other.end(), [&](const auto& s) { return s.first < end && r.pos < s.second; });
    });
  }

  // a rest starting where no note does gets a column of its own
  for (const auto& part_rests : found) {
    for (const auto& r : part_rests) {
      auto it = std::lower_bound(chords.begin(), chords.end(), r.pos,
                                 [](const auto& c, int pos) { return c.first < pos; });
      if (it == chords.end() || it->first != r.pos) {
        chords.insert(it, make_pair(r.pos, vector<sheetNote*>{}));
      }
    }
  }

  s_chordData.assign(chords.size(), sheetChordData{});
  for (int part = 0; part < part_count; ++part) {
    for (const auto& r : found[part]) {
      auto it = std::lower_bound(chords.begin(), chords.end(), r.pos,
                                 [](const auto& c, int pos) { return c.first < pos; });
      s_chordData[it - chords.begin()].rests[part] = r.type;
    }
  }

  // both voices resting alike share one rest in the usual place
  for (auto& cd : s_chordData) {
    for (int stave = STAVE_TREBLE; stave <= STAVE_BASS; ++stave) {
      int& upper = cd.rests[find_part(stave, 0)];
      int& lower = cd.rests[find_part(stave, 1)];
      if (upper >= 0 && upper == lower) {
        cd.rest_shared[stave] = true;
        lower = -1;
      }
    }
  }
}

void sheetMeasure::build_stem_groups(int len) {
  const midi& stream = ctr.getStream();

  // where each part plays next, so a stem never claims more time than its part leaves it
  vector<std::array<int, part_count>> next(chords.size());
  std::array<int, part_count> after;
  after.fill(len);
  for (int ch = static_cast<int>(chords.size()) - 1; ch >= 0; --ch) {
    next[ch] = after;
    for (const sheetNote* n : chords[ch].second) {
      if (n->stave != STAVE_NONE) {
        after[find_part(n->stave, n->voice)] = chords[ch].first;
      }
    }
  }

  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    const auto& c = chords[ch].second;
    for (int i = 0; i < static_cast<int>(c.size()); ++i) {
      const sheetNote* n = c[i];
      if (!n->visible || n->out_of_bounds || n->stave == STAVE_NONE) {
        continue;
      }

      stem_group& g = s_chordData[ch].groups[find_part(n->stave, n->voice)];
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
      // the stem shows the longest value on it that ends before its part plays again, so it meets the rests after
      // it, or the shortest when every value runs on (enum is inverted)
      const int room = next[ch][find_part(n->stave, n->voice)] - chords[ch].first;
      auto better = [&](int a, int b) {
        if ((a >= NOTE_HALF_DOT) != (b >= NOTE_HALF_DOT)) {
          return a >= NOTE_HALF_DOT;
        }
        if ((note_units[a] <= room) != (note_units[b] <= room)) {
          return note_units[a] <= room;
        }
        return note_units[a] <= room ? a < b : a > b;
      };
      if (i == g.first || better(n->display_type, g.type)) {
        g.type = n->display_type;
      }
      g.dotted |= is_dotted_type(n->display_type);
    }

    for (auto& g : s_chordData[ch].groups) {
      if (!g.empty()) {
        g.flag_type = getFlagType(g.type);
      }
    }
  }
}

bool sheetMeasure::is_plain_half_bar(int part, int half) const {
  // four eighths filling half of the bar may share one beam
  int count = 0;
  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    const int pos = chords[ch].first;
    if (pos < half * 32 || pos >= half * 32 + 32 || s_chordData[ch].groups[part].empty()) {
      continue;
    }
    if (s_chordData[ch].groups[part].type != NOTE_8 || pos != half * 32 + count * 8) {
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

  for (int part = 0; part < part_count; ++part) {
    const int stave = part_stave(part);
    const bool half_bar[2] = {common && is_plain_half_bar(part, 0), common && is_plain_half_bar(part, 1)};

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
          s_chordData[ch].groups[part].beam = beams.size();
        }
        beams.push_back({stave, part_voice(part), FLAG_UP, run, two_voice[stave], false, 0});
      }
      run.clear();
    };

    for (unsigned int ch = 0; ch < chords.size(); ++ch) {
      const stem_group& g = s_chordData[ch].groups[part];
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

void sheetMeasure::set_stem_dirs() {
  // in two voices the upper one points up and the lower one down, otherwise the pitch rule decides
  auto voice_dir = [](int voice) { return voice == 0 ? FLAG_UP : FLAG_DOWN; };

  for (auto& b : beams) {
    if (b.fixed) {
      b.dir = voice_dir(b.voice);
    }
    else {
      int lo = INT_MAX;
      int hi = INT_MIN;
      for (int ch : b.chords) {
        const stem_group& g = s_chordData[ch].groups[b.part()];
        lo = min(lo, g.lo);
        hi = max(hi, g.hi);
      }
      b.dir = find_stem_dir(lo, hi, b.stave);
    }
    for (int ch : b.chords) {
      s_chordData[ch].groups[b.part()].dir = b.dir;
      s_chordData[ch].groups[b.part()].fixed = b.fixed;
    }
  }

  for (auto& cd : s_chordData) {
    for (int part = 0; part < part_count; ++part) {
      stem_group& g = cd.groups[part];
      const int stave = part_stave(part);
      if (g.empty() || g.beam >= 0) {
        continue;
      }
      if (two_voice[stave]) {
        g.fixed = true;
        g.dir = voice_dir(part_voice(part));
      }
      else {
        // a whole note beside the stem still counts, so the stem points away from it
        g.dir = find_stem_dir(g.lo, g.hi, stave);
      }
    }
  }
}

void sheetMeasure::place_heads() {
  const midi& stream = ctr.getStream();

  for (unsigned int ch = 0; ch < chords.size(); ++ch) {
    auto& c = chords[ch].second;
    for (int part = 0; part < part_count; ++part) {
      stem_group& g = s_chordData[ch].groups[part];
      if (g.empty()) {
        continue;
      }
      const bool split = two_voice[part_stave(part)];

      int count = 0;
      for (int i = g.first; i <= g.last; ++i) {
        count += in_part(c[i], part);
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
        if (!in_part(n, part)) {
          continue;
        }

        const int y = stream.notes[n->oriNote].sheetY;
        const bool flip = prev_normal && abs(y - prev_y) == 1;
        n->left = down ? flip : !flip;
        g.flipped |= flip;
        prev_normal = !flip;
        prev_y = y;

        const int from_bottom = down ? count - 1 - k : k;
        if (split) {
          // each voice ties away from the other, around the heads only where no stem is in the way
          n->tie_up = part_voice(part) == 0;
          n->tie_outside = !g.has_stem() && (n->tie_up ? from_bottom == count - 1 : from_bottom == 0);
        }
        else {
          // outer notes tie away from the chord, a lone or middle note ties away from the stem
          n->tie_up = 2 * from_bottom == count - 1 ? down : 2 * from_bottom > count - 1;
          // lone notes and the outer notes of a chord tie around the heads, inner ones between them
          n->tie_outside =
              count == 1 || (from_bottom == count - 1 && n->tie_up) || (from_bottom == 0 && !n->tie_up);
        }
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
