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
using std::is_same;


class shaderData {
  public:

    shaderData(const asset& item);

    Shader& getShader() { return shader; }

    template <class T>
    void setShaderValue(const string& uf, const T& val) {
      // verify types
      static_assert(is_same<T, Vector2>::value || 
                    is_same<T, Vector3>::value || 
                    is_same<T, float>::value, 
                    "invalid type passed to uniform");

      auto it = typeMap.find(uf);
      if (it == typeMap.end()) {
        logW(LL_WARN, name, "- attempt to modify non-existent uniform:", uf);
        return;
      }
      SetShaderValue(shader, GetShaderLocation(shader, uf.c_str()), &val, it->second);
    }

    void unloadData();

  private:

    string name;
    
    Shader shader;
   
    // map uniform name to input type
    map<string, int> typeMap;

};
