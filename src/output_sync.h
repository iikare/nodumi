#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <set>
#include <thread>

#include "log.h"
#include "output.h"

using std::condition_variable;
using std::multiset;
using std::mutex;
using std::thread;

class outputInstance {
 public:
  void init(midiOutput* out);
  void terminate();

  void updateOffset(double off);
  void load(const multiset<pair<double, vector<unsigned char>>>& message);

  void allow();
  void disallow(bool force = false);

 private:
  void process();

  // these expect crit to be held
  size_t find_index(double off) const;
  void silence(bool all = false);

  // everything below is guarded by crit
  vector<pair<double, vector<unsigned char>>> message;
  midiOutput* output = nullptr;

  bool end = false;
  bool send = false;

  // false until the main thread gives an offset after allow() or load(), so nothing plays from a stale one
  bool anchored = false;

  // true for the frame after a seek, so holding a seek key stays quiet instead of playing at every stop
  bool settling = false;

  // channels that have played since the last silence
  unsigned int sounding = 0;

  // index of the next message to send, only the sender thread moves it forward during playback
  size_t index = 0;

  // last offset from the main thread and when it came, which the sender thread extrapolates from
  double offset = 0;
  std::chrono::time_point<std::chrono::steady_clock> update_last;

  // furthest offset the sender thread has played up to since the last seek
  double played = 0;

  mutex crit;
  condition_variable wake;
  thread oThread;
};
