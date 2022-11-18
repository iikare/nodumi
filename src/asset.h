#pragma once

#include "build_target.h"

#include <string>
#include "uniform.h"
#include "enum.h"
#include "log.h"

using std::string;

#define NAME(v) #v

class asset {
  public:
    asset(int a, const char* b, unsigned char* data, unsigned int dataLen) {
      assetType = a;
      assetName = b;
      this->data = data;
      this->dataLen = dataLen;
    }
    asset(int a, string b, unsigned char* data, vector<uniform> uniforms) {
      assetType = a;
      assetName = b;
      this->data = data;
      shaderUniforms = uniforms;
    }

    int assetType;
    string assetName;
    unsigned char* data;
    unsigned int dataLen;
    vector<uniform> shaderUniforms = {};
};
