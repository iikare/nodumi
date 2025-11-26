#include "output_sync.h"

#include <algorithm>
#include <chrono>

using std::distance;
using std::make_pair;

void outputInstance::init(midiOutput* out) {
  output = out;

  oThread = thread(&outputInstance::process, this);
}
void outputInstance::updateOffset(double off) {
  // crit.lock();
  // if (fabs(offset - this->offset) > std::numeric_limits<double>::epsilon()) {
  // interrupt = false;
  // return;
  //}
  auto update = std::chrono::high_resolution_clock::now();

  // logQ("provided offset", off);

  offset_last = offset;
  offset = off;

  double elapsed = std::chrono::duration<double>(update - update_last_overflow).count();
  if (fabs(offset - offset_last) > elapsed * UNK_CST + 1) {
    // logQ("real offset exceeds maximum possible offset:",
    // (offset-offset_last), elapsed*UNK_CST);

    // don't play too many notes at once
    // return;
  }

  // the overflow is strictly to stop too many notes from playing
  update_last = update;
  update_last_overflow = update;

  // send = offset > offset_last;
  // offset_last = offset;

  auto it = message.upper_bound(make_pair(off + (off <= 0 ? 0 : 1), vector<unsigned char>{}));
  // if (it != message.begin()) { it--; }

  int new_index = distance(message.begin(), it);
  // crit.lock();
  index_last = index;
  index = new_index;

  // crit.unlock();
  // logQ("lowest index:", index, "w/offset:", it->first, "v", offset);

  if (index != index_last) {
    // logQ("index_last v. index:", index_last, "v", index);
    if (index < index_last) {
      // logW(LL_WARN, "invalid direction");
    }
    // index_last = index;
  }

  // unsigned int index_last = index;
  // for (unsigned int idx = 0; idx < message.size(); ++idx) {
  // if (message[idx].first < 0.01) {
  ////logQ("alert");
  // continue;
  //}
  // if (message[idx].first >= offset) {
  // index = idx;//idx == 0 ? 0 : idx - 1;
  // break;
  //}
  //}

  // if (index_last > index) {
  // logQ("skipped note (backward):", index_last, index);
  ////index = min(static_cast<unsigned int>(index), index_last - 10);
  //}
  // if (index_last < index) {
  // logQ("skipped note (forward):", index_last, index);
  ////index = max(index_last, index - 10);
  // index = index_last;
  //}
  // else {
  // this->offset = offset;
  // last_update = std::chrono::high_resolution_clock::now();
  //}

  // crit.unlock();
}

void outputInstance::terminate() {
  end = true;
  oThread.join();
}

void outputInstance::process() {
  while (true) {
    if (end) {
      break;
    }

    if (send && !(offset == 0)) {
      crit.lock();
      std::chrono::duration<double> elapsedTime = std::chrono::high_resolution_clock::now() - update_last;

      // auto it_end =
      // message.upper_bound(make_pair(offset+elapsedTime.count()+1,
      // vector<unsigned char>{})); if (it_end != message.begin()) { it_end--; }

      double add_offset = elapsedTime.count() * UNK_CST + 0.5;

      auto it = std::next(message.begin(), index_last);
      double max_offset = offset_last + add_offset;

      // double offset =

      int up = 0;

      if (index_last < index || (it != message.end() && it->first < max_offset)) {
        // while (it != it_end) {

        vector<unsigned char>& msg = const_cast<vector<unsigned char>&>(it->second);
        // logQ("it. offset_last, limit offset_last:", it->first,
        // offset_last+elapsedTime.count()*UNK_CST);

        // logQ("/*msg*/ sent:", distance(message.begin(),it));
        // crit.lock();
        output->sendMessage(&msg);
        // crit.unlock();

        it++;
        up++;
      }

      index_last += up;
      crit.unlock();

      // crit.unlock();
      // index++;

      // update_last = std::chrono::high_resolution_clock::now();

      // logQ(elapsedTime);
    }
    else {
      std::this_thread::yield();
    }

    // logQ("hi from concurrent thread");
  }
}

void outputInstance::load(const multiset<pair<double, vector<unsigned char>>>& message) {
  crit.lock();
  this->message = message;
  crit.unlock();
}

void outputInstance::allow() {
  if (!send) {
    crit.lock();
    send = true;
    crit.unlock();
  }
}
void outputInstance::disallow(bool force) {
  if (send || force) {
    send = false;
    crit.lock();
    // all sound off, all note off
    vector<unsigned char> s = {120, 0, 123, 0};
    output->sendMessage(&s);
    crit.unlock();
  }
}
