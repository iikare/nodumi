#include "track_split.h"

#include "controller.h"
#include "define.h"

int findTrack(const note& n, const midi& m_stream, bool live, int numOn) {
  bool print = false;

  int n_start = 3250;
  int n_end = n_start + 100;
  print = true;
  if (n.number < n_start || n.number > n_end) {
    print = false;
  }

  constexpr int lookback_limit = 8;
  constexpr int duration_limit = 5 * UNK_CST;
  // constexpr int out_of_range_modifier = 100;
  // constexpr int empty_track_modifier = 10;

  int j = n.number - 1;
  int j_i = 0;

  double track0_sum = 0;
  double track1_sum = 0;
  int track0_ct = 0;
  int track1_ct = 0;

  int track0_on_min = 127;
  int track1_on_min = 127;

  int track0_on_max = 0;
  int track1_on_max = 0;

  int temporal_ct0 = 0;
  int temporal_ct1 = 0;

  // auto last_onset = n.x;
  while (j >= 0 && j_i < lookback_limit) {
    // if (last_onset - m_stream.notes[j].x > 1000) break;

    if (m_stream.notes[j].track) {
      track1_sum += m_stream.notes[j].y;
      track1_ct++;
      if (abs(m_stream.notes[j].x - n.x) < duration_limit) {
        track1_on_min = min(track1_on_min, m_stream.notes[j].y);
        track1_on_max = max(track1_on_max, m_stream.notes[j].y);
        temporal_ct1++;
      }
    }
    else {
      track0_sum += m_stream.notes[j].y;
      track0_ct++;
      if (abs(m_stream.notes[j].x - n.x) < duration_limit) {
        track0_on_min = min(track0_on_min, m_stream.notes[j].y);
        track0_on_max = max(track0_on_max, m_stream.notes[j].y);
        temporal_ct0++;
      }
    }

    j--;
    j_i++;
  }

  double track0_avg = 48;
  double track1_avg = 72;

  if (j_i) {
    if (track0_ct) {
      track0_avg = track0_sum / track0_ct;
    }
    if (track1_ct) {
      track1_avg = track1_sum / track1_ct;
    }
  }

  double score0 = abs(track0_avg - n.y);
  double score1 = abs(track1_avg - n.y);

  int track0_range = track0_on_max - track0_on_min;
  int track1_range = track1_on_max - track1_on_min;
  int track0_range_new = max(track0_on_max, n.y) - min(track0_on_min, n.y);
  int track1_range_new = max(track1_on_max, n.y) - min(track1_on_min, n.y);
  if (temporal_ct0 == 0) {
    track0_range = 0;
    track0_range_new = 0;
  }
  if (temporal_ct1 == 0) {
    track1_range = 0;
    track1_range_new = 0;
  }

  if (track0_range <= ctr.option.get(OPTION::HAND_RANGE) &&
      track0_range_new > ctr.option.get(OPTION::HAND_RANGE)) {
    // score0 += out_of_range_modifier;
  }

  if (track1_range <= ctr.option.get(OPTION::HAND_RANGE) &&
      track1_range_new > ctr.option.get(OPTION::HAND_RANGE)) {
    // score1 += out_of_range_modifier;
  }

  int track_result = score0 > score1;

  if (print) {
    logQ(n.number, track0_range, track0_range_new, track1_range, track1_range_new);
  }
  if (print) {
    logQ(n.number, track_result, track0_avg, track1_avg, temporal_ct0, temporal_ct1, n.y);
  }

  return track_result;
}
