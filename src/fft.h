#pragma once

#include <cstddef>
#include <utility>
#include <vector>

using std::pair;
using std::vector;

// a modelled grand piano spectrum, built from the notes sounding at the song position
class fftController {
 public:
  // bars are 10 px apart across the window, log spaced over the audible range
  static constexpr int bin_spacing = 10;

  struct bin_part {
    int note;
    float share;
  };

  // forgets the note order and levels, for a newly loaded file
  void reset();

  // moves the spectrum to the song position, running is false while paused and dt is the frame length in
  // seconds
  void update(double offset, bool running, double dt);

  int get_bin_count() const { return bin_count; }

  // shown height of each bar and of its peak marker, from 0 to 1
  const vector<float>& get_heights() const { return heights; }
  const vector<float>& get_peaks() const { return peaks; }

  // the notes making up each bar from the bottom, highest track first and then oldest first, with their share
  // of its height
  const vector<vector<bin_part>>& get_parts() const { return shown_parts; }

 private:
  void index_notes();
  void add_note(int idx, double t);
  double release_time(int idx) const;

  int bin_count = 0;

  // note indices sorted by start, since notes are stored track by track
  vector<int> order;
  const void* indexed_data = nullptr;
  size_t indexed_count = 0;

  // sustain pedal changes as (time, down), in song time
  vector<pair<double, bool>> pedal;

  vector<float> power;
  vector<vector<bin_part>> parts;
  vector<vector<bin_part>> shown_parts;
  vector<float> level_db;
  vector<float> peak_db;
  vector<float> peak_hold;
  vector<float> heights;
  vector<float> peaks;
  double last_offset = -1;
};
