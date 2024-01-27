#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>

#include "log.h"
#include "output.h"

using std::atomic;
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
  void disallow();

 private:
  void process();

  multiset<pair<double, vector<unsigned char>>> message;
  midiOutput* output;

  bool end = false;
  atomic<bool> send = false;

  unsigned int index = 0;
  unsigned int index_last = 0;

  double offset = 0;
  double offset_last = 0;

  std::chrono::time_point<std::chrono::high_resolution_clock> update_last;
  std::chrono::time_point<std::chrono::high_resolution_clock> update_last_overflow;

  mutex crit;
  thread oThread;
};
