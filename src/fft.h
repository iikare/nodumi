#pragma once

#include <vector>
#include "log.h"


using std::vector;
using std::make_pair;

class fftController {
  public:

    double getFundamental(int y);
    double fftAC(double f_1, double f_2);
    
    vector<pair<double, int>> fftbins;

    static constexpr double harmonics[5] = {0.25, 0.5, 1, 2, 4};
    static constexpr double harmonicsCoefficient[5] = {0.04, 0.1, 1, 0.1, 0.04};
    static constexpr int harmonicsSize = 5;
    
    friend class controller;

  private: 
    void updateFFTBins();
};
