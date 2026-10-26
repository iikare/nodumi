#include "fft.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include "define.h"

using std::clamp;
using std::max;

namespace {

// the bars cover what can be heard
constexpr double low_hz = 20;
constexpr double high_hz = 20000;
constexpr int max_partials = 32;

// shown range in dB, from a fortissimo note's strongest partial, with headroom for notes adding up
constexpr double floor_db = -60;
constexpr double ceiling_db = 6;
// the quietest partial worth adding, at the floor
constexpr double floor_amp = 1e-3;

// bars rise at once but fall no faster than this
constexpr double fall_db_per_s = 80;
// peak markers hold for a moment, then drop back onto the bars
constexpr double peak_hold_s = 0.1;
constexpr double peak_fall_db_per_s = 100;
// a slow shimmer on sounding bars, so held chords stay alive
constexpr double shimmer_db = 1.5;
constexpr double shimmer_hz = 3;

// a released key's damper stops the string within about half a second
constexpr double damper_tau_s = 0.07;
// grand pianos have no dampers from about F#6 up
constexpr int undamped_key = 90;
// notes older than this have died away, even held in the bass
constexpr double audible_s = 50;
// each partial spreads as a gaussian at least this wide in bars, wider in the bass where a fixed frequency
// resolution spans several bars, as on a real analyzer
constexpr double spread_min_bins = 1.5;
constexpr double spread_hz = 6;

double key_hz(int key) { return 440 * std::pow(2.0, (key - 69) / 12.0); }

// smooth noise in [-1, 1] for each bin over song time, the same on every run and at any frame rate
double shimmer(int bin, double seconds) {
  const auto hash = [bin](int64_t step) {
    uint64_t x = static_cast<uint64_t>(step) * 0x9E3779B97F4A7C15ull;
    x += static_cast<uint64_t>(bin + 1) * 0xBF58476D1CE4E5B9ull;
    x ^= x >> 31;
    x *= 0x94D049BB133111EBull;
    x ^= x >> 29;
    return static_cast<double>(x >> 11) / static_cast<double>(1ull << 53) * 2 - 1;
  };
  const double phase = seconds * shimmer_hz;
  const int64_t step = static_cast<int64_t>(std::floor(phase));
  const double f = phase - step;
  const double a = hash(step);
  return a + (hash(step + 1) - a) * f * f * (3 - 2 * f);
}

}  // namespace

void fftController::reset() {
  // a count no note list has, so the next update indexes whatever is loaded
  indexed_count = std::numeric_limits<size_t>::max();
  last_offset = -1;

  power.assign(bin_count, 0);
  parts.assign(bin_count, {});
  shown_parts.assign(bin_count, {});
  level_db.assign(bin_count, floor_db - 1);
  peak_db.assign(bin_count, floor_db - 1);
  peak_hold.assign(bin_count, 0);
  heights.assign(bin_count, 0);
  peaks.assign(bin_count, 0);
}

void fftController::index_notes() {
  const vector<note>& notes = ctr.getNotes();
  indexed_data = notes.data();
  indexed_count = notes.size();

  order.resize(notes.size());
  for (size_t i = 0; i < order.size(); i++) {
    order[i] = i;
  }
  std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return notes[a].x < notes[b].x; });

  pedal.clear();
  bool down = false;
  for (const auto& [time, msg] : ctr.getStream().message) {
    if (msg.size() >= 3 && (msg[0] & 0xF0) == 0xB0 && msg[1] == 64 && (msg[2] >= 64) != down) {
      down = msg[2] >= 64;
      pedal.push_back({time, down});
    }
  }
}

double fftController::release_time(int idx) const {
  const note& n = ctr.getNotes()[idx];
  constexpr double never = std::numeric_limits<double>::infinity();
  if (n.isOn || n.y >= undamped_key) {
    return never;
  }

  // a sustain pedal held when the key comes up keeps the damper off until the pedal lifts
  double end = n.x + n.duration;
  auto next = std::upper_bound(pedal.begin(), pedal.end(), end,
                               [](double t, const pair<double, bool>& change) { return t < change.first; });
  if (next != pedal.begin() && std::prev(next)->second) {
    if (next == pedal.end()) {
      return never;
    }
    end = next->first;
  }
  return (end - n.x) / UNIT_TPS;
}

void fftController::add_note(int idx, double t) {
  const note& n = ctr.getNotes()[idx];
  const double vel = clamp(n.velocity, 1, 127) / 127.0;
  const double reg = clamp((n.y - 21) / 87.0, 0.0, 1.0);

  // a held note dies away in about a minute in the deep bass, 15 s around middle C and 3 s at the top,
  // quickly at first and then slower
  const double tau = 60 * std::pow(0.05, reg) / 6.91;
  const double released = release_time(idx);
  const double damping = t > released ? std::exp(-(t - released) / damper_tau_s) : 1;
  // 40 log10 of the velocity in dB, so pianissimo to fortissimo spans about 30 dB
  const double level = vel * vel * damping;
  if (level * (0.7 * std::exp(-t * 8 / tau) + 0.3 * std::exp(-t / tau)) < floor_amp) {
    return;
  }

  // bass notes are rich and treble notes nearly pure, and a harder strike is brighter
  const double slope = 1.0 + 0.8 * reg - 0.5 * vel;
  // stiff strings sharpen the upper partials, more so in the treble
  const double stiffness = 4e-4 * std::pow(10.0, (n.y - 60) * 0.02);
  const double f0 = key_hz(n.y);
  const double bins_per_log = (bin_count - 1) / std::log(high_hz / low_hz);

  for (int p = 1; p <= max_partials; p++) {
    const double hz = p * f0 * std::sqrt(1 + stiffness * p * p);
    if (hz > high_hz) {
      break;
    }
    // higher partials die away faster
    const double tau_p = tau / std::pow(p, 0.7);
    const double env = 0.7 * std::exp(-t * 8 / tau_p) + 0.3 * std::exp(-t / tau_p);
    // the soundboard radiates little below about 70 Hz, and the highs roll off above 5 kHz
    const double radiation = hz * hz / (hz * hz + 70 * 70) / std::sqrt(1 + std::pow(hz / 5000, 4));
    const double amp = level * std::pow(p, -slope) * radiation * env;
    if (amp < floor_amp) {
      continue;
    }

    // the whole gaussian, out to where it sinks below the floor, keeping the partial's level at its center
    const double pos = std::log(hz / low_hz) * bins_per_log;
    const double sigma = max(spread_min_bins, spread_hz * bins_per_log / hz);
    const double reach = 2 * sigma * std::sqrt(std::log(amp / floor_amp));
    const int first = max(0, static_cast<int>(std::ceil(pos - reach)));
    const int last = std::min(bin_count - 1, static_cast<int>(std::floor(pos + reach)));
    for (int bin = first; bin <= last; bin++) {
      const double d = (bin - pos) / sigma;
      const float part_power = amp * amp * std::exp(-0.5 * d * d);
      power[bin] += part_power;
      // a note's partials all go in before the next note's, so its share of a bin is the bin's latest entry
      if (!parts[bin].empty() && parts[bin].back().note == idx) {
        parts[bin].back().share += part_power;
      }
      else {
        parts[bin].push_back({idx, part_power});
      }
    }
  }
}

void fftController::update(double offset, bool running, double dt) {
  // a resize changes how many bars fit, which starts the levels over
  const int count = max(2, ctr.getWidth() / bin_spacing - 2);
  if (count != bin_count || level_db.size() != static_cast<size_t>(count)) {
    bin_count = count;
    reset();
  }
  const vector<note>& notes = ctr.getNotes();
  if (notes.data() != indexed_data || notes.size() != indexed_count) {
    index_notes();
  }

  for (int bin = 0; bin < bin_count; bin++) {
    power[bin] = 0;
    parts[bin].clear();
  }

  // sitting still while paused lets everything ring down, like the rings in PULSE
  const bool still = !running && offset == last_offset;
  last_offset = offset;
  if (!still) {
    const double oldest = offset - audible_s * UNIT_TPS;
    auto it =
        std::lower_bound(order.begin(), order.end(), oldest, [&](int i, double t) { return notes[i].x < t; });
    for (; it != order.end() && notes[*it].x <= offset; ++it) {
      add_note(*it, (offset - notes[*it].x) / UNIT_TPS);
    }
  }

  const double seconds = offset / UNIT_TPS;
  for (int bin = 0; bin < bin_count; bin++) {
    double target = floor_db - 1;
    if (power[bin] > 0) {
      target = 10 * std::log10(power[bin]) + shimmer_db * shimmer(bin, seconds);
      for (auto& part : parts[bin]) {
        part.share /= power[bin];
      }
      // stacked by track, so each track keeps its place in the bar whichever one played last
      std::stable_sort(parts[bin].begin(), parts[bin].end(), [&](const bin_part& a, const bin_part& b) {
        return notes[a.note].track > notes[b.note].track;
      });
      shown_parts[bin] = parts[bin];
    }

    level_db[bin] = max(target, level_db[bin] - fall_db_per_s * dt);

    if (level_db[bin] >= peak_db[bin]) {
      peak_db[bin] = level_db[bin];
      peak_hold[bin] = peak_hold_s;
    }
    else if (peak_hold[bin] > 0) {
      peak_hold[bin] -= dt;
    }
    else {
      peak_db[bin] = max<float>(level_db[bin], peak_db[bin] - peak_fall_db_per_s * dt);
    }
    // the peak marker outlasts the bar, and takes its color from the bar's notes
    if (peak_db[bin] <= floor_db) {
      shown_parts[bin].clear();
    }

    heights[bin] = clamp((level_db[bin] - floor_db) / (ceiling_db - floor_db), 0.0, 1.0);
    peaks[bin] = clamp((peak_db[bin] - floor_db) / (ceiling_db - floor_db), 0.0, 1.0);
  }
}
