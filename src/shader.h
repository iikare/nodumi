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
using std::to_string;
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

      constexpr int ufType = is_same<T, Vector2>::value ? SHADER_UNIFORM_VEC2  :
                             is_same<T, Vector3>::value ? SHADER_UNIFORM_VEC3  :
                             is_same<T, float>::value   ? SHADER_UNIFORM_FLOAT : 255; // unused value

      auto it = typeMap.find(uf);
      if (it == typeMap.end()) {
        logW(LL_WARN, name, "- attempt to modify non-existent uniform:", uf);
        return;
      }
      if (ufType != it->second) {
        logW(LL_WARN, name, "- expecting uniform of type", to_string(it->second)+",", "not", ufType);
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
