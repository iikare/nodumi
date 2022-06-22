#pragma once

#include <string>
#include "enum.h"
#include "log.h"

using std::string;

#define NAME(v) #v

class asset {
  public:
    asset(int a, string b, unsigned char* data, unsigned int dataLen) {
      assetType = a;
      assetName = b;
      this->data = data;
      this->dataLen = dataLen;
    }

    int assetType;
    string assetName;
    unsigned char* data;
    unsigned int dataLen;
};
