#pragma once

#include <vector>

using std::vector;
using std::pair;
using std::make_pair;

class sheetMeasure {
  public:

    void reset() {
      events.clear();
      spacing.clear();
    }

  private:
    vector<int> events;
    vector<int> spacing;

};
