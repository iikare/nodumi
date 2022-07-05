#pragma once

#include <vector>
#include "enum.h"

using std::vector;
using std::pair;
using std::make_pair;

class sheetMeasure {
  public:

    void reset() {
      events.clear();
      spacing.clear();
    }

    void addEvent(int eventType);
    void addSpace(int space);

  private:
    vector<int> events;
    vector<int> spacing;

};
