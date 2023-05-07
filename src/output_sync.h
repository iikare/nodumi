#pragma once

#include <chrono>
#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include "output.h"
#include "log.h"

using std::multiset;
using std::thread;
using std::mutex;
using std::atomic;

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
    //atomic<double> offset;
    //atomic<double> offset_last;
    //atomic<unsigned int> index;
    //atomic<bool> interrupt;
    //atomic<bool> interrupt_ack;
    //atomic<bool> end;
    //atomic<bool> send;
    
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

