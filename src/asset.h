#pragma once

#include <string>

#include "build_target.h"
#include "enum.h"
#include "log.h"
#include "uniform.h"

using std::string;

#define NAME(v) #v

class asset {
 public:
  asset(ASSET a, const char* b, unsigned char* data, unsigned int dataLen) {
    assetType = a;
    assetName = b;
    this->data = data;
    this->dataLen = dataLen;
  }
  asset(ASSET a, string b, unsigned char* data1, unsigned char* data2, vector<uniform> uniforms) {
    assetType = a;
    assetName = b;
    dataVS = data1;
    dataFS = data2;
    shaderUniforms = uniforms;
  }

  ASSET assetType;
  string assetName;
  unsigned char* dataVS;
  unsigned char* dataFS;
  unsigned char* data;
  unsigned int dataLen;
  vector<uniform> shaderUniforms = {};
};
