#pragma once

#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include "output.h"
#include "log.h"

using std::vector;
using std::thread;
using std::atomic;

class outputInstance {
  public:

    outputInstance() {
      end = false;
      interrupt = false;
      interrupt_ack = false;
      send = false;
      index = 0;
    }

    void init(midiOutput* out);
    void terminate();

    void updateOffset(double offset);
    void load(const vector<pair<double, vector<unsigned char>>>& message);

    void allow();
    void disallow();

  private:
    
    void process();

    vector<pair<double, vector<unsigned char>>> message;
    midiOutput* output;
    atomic<double> offset;
    atomic<unsigned int> index;
    atomic<bool> interrupt;
    atomic<bool> interrupt_ack;
    atomic<bool> end;
    atomic<bool> send;
    thread oThread;

    std::chrono::time_point<std::chrono::high_resolution_clock> last_update;



    
};

