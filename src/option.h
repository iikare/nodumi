#pragma once

#include <filesystem>
#include <vector>

#include "enum.h"

class optionController {
 public:
  optionController();

  void invert(OPTION opt);

  bool invalid(OPTION opt);

  int get(OPTION opt);
  void set(OPTION opt, int value);

 private:
  vector<int> opts;
};
