#pragma once

#include <string>

using std::string;

struct uniform {
  uniform(const char* name, int type) {
    this->type = type;
    this->name = name;
  }
    
  int type;
  string name;
};
