#pragma once

#include <vector>
#include "enum.h"

using std::vector;
using std::pair;
using std::make_pair;

class sheetMeasure {
  public:

    void reset() {
      spacing.clear();
    }

    void addSpace(int space);

  private:
    vector<int> spacing;

};
