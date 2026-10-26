#include "output_sync.h"

#include <algorithm>

using std::lock_guard;
using std::unique_lock;

// messages this close to due are sent early, to cover the sender thread waking late
constexpr double send_lead = 0.001 * UNIT_TPS;
// a jump past this is a seek, anything smaller is frame jitter or arrow key scrubbing that keeps playing
constexpr double seek_limit = 0.1 * UNIT_TPS;

void outputInstance::init(midiOutput* out) {
  output = out;

  oThread = thread(&outputInstance::process, this);
}

void outputInstance::updateOffset(double off) {
  auto now = std::chrono::steady_clock::now();

  lock_guard<mutex> lock(crit);
  if (send && anchored) {
    double expect = offset + std::chrono::duration<double>(now - update_last).count() * UNIT_TPS;
    // after a stall off can trail what was played for a frame, so only a drop in off itself is a seek back
    bool back = off < offset && (off < offset - seek_limit || played > off + seek_limit);
    bool forward = off > std::max(expect, played) + seek_limit;
    if (back || forward) {
      silence();
      index = find_index(off);
      played = off;
    }
    settling = back || forward;
  }
  else {
    index = find_index(off);
    played = off;
    settling = false;
  }

  anchored = true;
  offset = off;
  update_last = now;
  wake.notify_one();
}

void outputInstance::terminate() {
  {
    lock_guard<mutex> lock(crit);
    if (send) {
      send = false;
      silence();
    }
    end = true;
  }
  wake.notify_one();
  if (oThread.joinable()) {
    oThread.join();
  }
}

void outputInstance::process() {
  unique_lock<mutex> lock(crit);
  while (!end) {
    if (!send || !anchored || settling || index >= message.size()) {
      wake.wait(lock);
      continue;
    }

    double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - update_last).count();
    double due = offset + elapsed * UNIT_TPS + send_lead;
    played = std::max(played, due);
    while (index < message.size() && message[index].first <= due) {
      output->sendMessage(&message[index].second);
      sounding |= 1u << (message[index].second[0] & 0x0F);
      index++;
    }

    // sleep until the next message is due, any change from the main thread wakes this early
    if (index < message.size()) {
      wake.wait_for(lock, std::chrono::duration<double>((message[index].first - due) / UNIT_TPS));
    }
  }
}

void outputInstance::load(const multiset<pair<double, vector<unsigned char>>>& message) {
  lock_guard<mutex> lock(crit);
  this->message.assign(message.begin(), message.end());
  index = 0;
  anchored = false;
}

void outputInstance::allow() {
  lock_guard<mutex> lock(crit);
  if (!send) {
    send = true;
    anchored = false;
  }
}

void outputInstance::disallow(bool force) {
  lock_guard<mutex> lock(crit);
  if (send || force) {
    send = false;
    silence(force);
  }
}

size_t outputInstance::find_index(double off) const {
  auto it = std::lower_bound(message.begin(), message.end(), off,
                             [](const pair<double, vector<unsigned char>>& m, double t) { return m.first < t; });
  return std::distance(message.begin(), it);
}

void outputInstance::silence(bool all) {
  // all sound off and all notes off, as one message each for backends that can't take more
  for (unsigned char ch = 0; ch < 16; ch++) {
    if (!all && !(sounding >> ch & 1)) {
      continue;
    }
    vector<unsigned char> sound_off = {static_cast<unsigned char>(0xB0 | ch), 120, 0};
    vector<unsigned char> notes_off = {static_cast<unsigned char>(0xB0 | ch), 123, 0};
    output->sendMessage(&sound_off);
    output->sendMessage(&notes_off);
  }
  sounding = 0;
}
