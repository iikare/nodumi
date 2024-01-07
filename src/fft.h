#pragma once

#include "build_target.h"

#include <vector>
#include <thread>
#include <mutex>
#include "log.h"

using std::vector;
using std::thread;
using std::mutex;

class fftController {
  public:

    double getFundamental(int y);
    double fftAC(double f_1, double f_2);
    void generateFFTBins(const vector<int> c_note, double offset);
    vector<vector<pair<int, int>>>& getFFTBins();

    void generator_join();

    
    vector<pair<double, int>> bins;

    static constexpr double harmonics[5] = {0.25, 0.5, 1, 2, 4};
    static constexpr double harmonicsCoefficient[5] = {0.04, 0.1, 1, 0.1, 0.04};
    static constexpr int harmonicsSize = 5;
    
    friend class controller;

  private: 

    // map fftbin index to (note index, length)
    vector<vector<pair<int, int>>> bin_map;

    const vector<int> c_note_last;
    vector<vector<pair<int, int>>> bin_map_last;

    mutex crit;
    thread generator;

    void updateFFTBins();
    void generate(const vector<int> c_note, double offset);
};
