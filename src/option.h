#pragma once

#include <vector>
#include "enum.h"


class optionController {
  public:

    optionController();

    void invert(optionType opt);

    bool get(optionType opt);

  private:

    vector<bool> opts;
};
