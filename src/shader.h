#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include <map>
#include <string>
#include <vector>
#include "asset.h"
#include "uniform.h"

using std::map;
using std::vector;
using std::string;


class shaderData {
  public:

    shaderData(const asset& item);

    Shader& getShader() { return shader; }

    template <class T>
    void setShaderValue(string& uf, T& val);
       

    void unloadData();

  private:

    string name;
    
    Shader shader;
   
    // map uniform name to both (a) location, (b) input type
    map<string, pair<int, int>> typeMap;

};
