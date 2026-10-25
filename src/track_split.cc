#include "track_split.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "controller.h"
#include "define.h"

using std::pair;
using std::vector;

namespace {

// times are in units of 1 / UNIT_TPS seconds
constexpr double chord_gap = 0.03 * UNIT_TPS;  // onsets closer than this belong to one chord
constexpr int chord_limit = 10;                // two hands of five fingers
constexpr double lookback_time = 8.0 * UNIT_TPS;
constexpr int lookback_limit = 64;
constexpr double position_decay = 3.0 * UNIT_TPS;
constexpr double legato_grace = 0.03 * UNIT_TPS;  // shorter overlaps count as legato, not held notes
constexpr int recent_limit = 2;
constexpr int max_fingers = 5;

// file input can see upcoming notes, so it searches a few chords ahead
constexpr int beam_width = 8;
constexpr int beam_depth = 6;
constexpr int path_limit = 64;

constexpr double default_center[2] = {48, 72};
constexpr double default_weight = 0.5;

constexpr double w_center = 0.5;
constexpr double w_near = 1.0;
constexpr double w_spread = 0.75;
constexpr double w_span = 2.0;
constexpr double span_base = 6.0;
constexpr double w_count = 20.0;
constexpr double w_cross = 4.0;

// a note before the chord, weighted by how long ago it was struck
struct past_note {
  int index;
  int y;
  double weight;
  bool held;
};

struct hand_state {
  double w_sum = 0;
  double wy_sum = 0;
  int recent[recent_limit];  // newest first
  int recent_ct = 0;
  int lo = 127;  // pitch bounds of notes down at the chord onset
  int hi = 0;
  int ct = 0;

  void hold(int y) {
    lo = std::min(lo, y);
    hi = std::max(hi, y);
    ct++;
  }

  void add(const past_note& p) {
    w_sum += p.weight;
    wy_sum += p.weight * p.y;
    if (recent_ct < recent_limit) {
      recent[recent_ct++] = p.y;
    }
    if (p.held) {
      hold(p.y);
    }
  }

  // older notes only fill what is left of the recent list
  void add_older(const hand_state& older) {
    w_sum += older.w_sum;
    wy_sum += older.wy_sum;
    for (int k = 0; k < older.recent_ct && recent_ct < recent_limit; k++) {
      recent[recent_ct++] = older.recent[k];
    }
    lo = std::min(lo, older.lo);
    hi = std::max(hi, older.hi);
    ct += older.ct;
  }

  double position_cost(int hand, int y) const {
    double center = (wy_sum + default_weight * default_center[hand]) / (w_sum + default_weight);
    double near = std::abs(y - center);
    for (int k = 0; k < recent_ct; k++) {
      near = std::min(near, static_cast<double>(std::abs(y - recent[k])));
    }
    return w_center * std::abs(y - center) + w_near * near;
  }
};

struct search_path {
  double cost = 0;
  signed char track[path_limit] = {};  // hands of the notes from the requested note onward
};

// free notes are sorted by pitch, and those below the split go to the left hand
double split_cost(const hand_state hand[2], const vector<pair<int, int>>& free_notes, int split,
                  int hand_range) {
  double cost = 0;
  int lo[2] = {hand[0].lo, hand[1].lo};
  int hi[2] = {hand[0].hi, hand[1].hi};
  int ct[2] = {hand[0].ct, hand[1].ct};

  for (int k = 0; k < static_cast<int>(free_notes.size()); k++) {
    int h = k >= split;
    int y = free_notes[k].first;
    cost += hand[h].position_cost(h, y);
    lo[h] = std::min(lo[h], y);
    hi[h] = std::max(hi[h], y);
    ct[h]++;
  }

  for (int h = 0; h < 2; h++) {
    if (!ct[h]) {
      continue;
    }
    cost += w_spread * (hi[h] - lo[h]);
    if (hi[h] - lo[h] > hand_range) {
      cost += span_base + w_span * (hi[h] - lo[h] - hand_range);
    }
    if (ct[h] > max_fingers) {
      cost += w_count * (ct[h] - max_fingers);
    }
  }

  if (ct[0] && ct[1] && hi[0] > lo[1]) {
    cost += w_cross * (hi[0] - lo[1]);
  }

  return cost;
}

}  // namespace

int findTrack(const note& n, const midi& m_stream) {
  const auto& notes = m_stream.notes;
  const int hand_range =
      ctr.option.get(OPTION::SET_HAND_RANGE) ? ctr.option.get(OPTION::HAND_RANGE) : MAX_HAND_RANGE;
  const int note_ct = static_cast<int>(notes.size());
  const int first = n.number;

  // notes before n already have hands, notes after n only exist for file input and do not
  int g_begin = first;
  while (g_begin > 0 && first - g_begin < chord_limit - 1 &&
         notes[g_begin].x - notes[g_begin - 1].x < chord_gap) {
    g_begin--;
  }

  vector<search_path> beam(1);
  vector<search_path> next;
  beam.reserve(beam_width * (chord_limit + 1));
  next.reserve(beam_width * (chord_limit + 1));
  vector<pair<int, int>> free_notes;  // pitch, index
  vector<past_note> past;

  // decay weights relative to the first chord, rescaled for later ones
  const int w_begin = std::max(0, g_begin - lookback_limit);
  const double first_onset = notes[g_begin].x;
  vector<double> weight;

  for (int depth = 0; depth < beam_depth && g_begin < note_ct; depth++) {
    int g_end = std::max(g_begin, first) + 1;
    while (g_end < note_ct && g_end - first < path_limit && g_end - g_begin < chord_limit &&
           notes[g_end].x - notes[g_end - 1].x < chord_gap) {
      g_end++;
    }

    while (w_begin + static_cast<int>(weight.size()) < g_begin) {
      weight.push_back(std::exp(-(first_onset - notes[w_begin + weight.size()].x) / position_decay));
    }

    // newest first, so notes decided along the search paths lead
    const double onset = notes[g_begin].x;
    const double scale = std::exp(-(onset - first_onset) / position_decay);
    past.clear();
    for (int j = g_begin - 1;
         j >= w_begin && g_begin - j <= lookback_limit && onset - notes[j].x <= lookback_time; j--) {
      const note& p = notes[j];
      bool held = p.isOn || p.x + p.duration > onset + legato_grace;
      past.push_back({j, p.y, weight[j - w_begin] * scale, held});
    }

    // notes with final hands look the same from every path
    hand_state settled[2];
    int path_past_ct = 0;
    for (const past_note& p : past) {
      if (p.index >= first) {
        path_past_ct++;
      }
      else {
        settled[notes[p.index].track != 0].add(p);
      }
    }
    for (int j = g_begin; j < first; j++) {
      settled[notes[j].track != 0].hold(notes[j].y);
    }

    free_notes.clear();
    for (int j = std::max(g_begin, first); j < g_end; j++) {
      free_notes.push_back({notes[j].y, j});
    }
    std::sort(free_notes.begin(), free_notes.end());
    const int free_ct = static_cast<int>(free_notes.size());

    next.clear();
    for (const search_path& path : beam) {
      hand_state hand[2];
      for (int k = 0; k < path_past_ct; k++) {
        hand[path.track[past[k].index - first]].add(past[k]);
      }
      hand[0].add_older(settled[0]);
      hand[1].add_older(settled[1]);

      for (int split = 0; split <= free_ct; split++) {
        search_path& branch = next.emplace_back(path);
        branch.cost += split_cost(hand, free_notes, split, hand_range);
        for (int k = 0; k < free_ct; k++) {
          branch.track[free_notes[k].second - first] = k >= split;
        }
      }
    }

    const int keep = std::min(beam_width, static_cast<int>(next.size()));
    std::partial_sort(next.begin(), next.begin() + keep, next.end(),
                      [](const auto& a, const auto& b) { return a.cost < b.cost; });
    next.resize(keep);
    beam.swap(next);
    g_begin = g_end;
    if (g_begin - first >= path_limit) {
      break;
    }
  }

  return beam[0].track[0];
}
