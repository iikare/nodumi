#pragma once

#include <vector>
#include "enum.h"

class optionController {
  public:
    optionController();

    void invert(optionType opt);

    bool invalid(optionType opt);

    int get(optionType opt);
    void set(optionType opt, int value);
  private:
    vector<int> opts;
};
