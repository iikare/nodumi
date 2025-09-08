#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "asset.h"
#include "build_target.h"
#include "color.h"
#include "uniform.h"

using std::is_same;
using std::map;
using std::min;
using std::string;
using std::to_string;
using std::vector;

class shaderData {
 public:
  shaderData(const asset& item);

  Shader& getShader() { return shader; }

  void setShaderTexture(const string& uf, const RenderTexture& rtex);

  template <class T>
  void setShaderValue(const string& uf, const T& val, const int num = -1) {
    // verify types
    static_assert(is_same<T, Vector2>::value || is_same<T, Vector3>::value || is_same<T, colorRGB>::value ||
                      is_same<T, float>::value || is_same<T, int>::value ||
                      is_same<T, vector<Vector2>>::value || is_same<T, vector<Vector3>>::value ||
                      is_same<T, vector<colorRGB>>::value || is_same<T, vector<float>>::value ||
                      is_same<T, vector<int>>::value,
                  "invalid type passed to uniform");

    constexpr int ufType =
        is_same<T, Vector2>::value || is_same<T, vector<Vector2>>::value   ? SHADER_UNIFORM_VEC2
        : is_same<T, Vector3>::value || is_same<T, vector<Vector3>>::value ? SHADER_UNIFORM_VEC3
                                                                           :
                                                                           // cast to float
            is_same<T, colorRGB>::value || is_same<T, vector<colorRGB>>::value ? SHADER_UNIFORM_VEC3
        : is_same<T, float>::value || is_same<T, vector<float>>::value         ? SHADER_UNIFORM_FLOAT
                                                                               :
                                                                       // 255 - unused value
            is_same<T, int>::value || is_same<T, vector<int>>::value ? SHADER_UNIFORM_INT
                                                                     : 255;

    constexpr bool isVecType = is_same<T, vector<Vector2>>::value || is_same<T, vector<Vector3>>::value ||
                               is_same<T, vector<colorRGB>>::value || is_same<T, vector<float>>::value ||
                               is_same<T, vector<int>>::value;

    constexpr bool isCRGB = is_same<T, colorRGB>::value || is_same<T, vector<colorRGB>>::value;

    auto it = typeMap.find(uf);
    if (it == typeMap.end()) {
      logW(LL_WARN, name, "- attempt to modify non-existent uniform:", uf);
      return;
    }
    if (ufType != it->second) {
      logW(LL_WARN, name, "- expecting uniform of type", to_string(it->second) + ",", "not", ufType);
      return;
    }

    if (isVecType) {
      if (num != -1 && num <= 0) {
        logW(LL_WARN, name, "- invalid uniform array passed with size", num);
        return;
      }
    }
    else {
      if (num != -1) {
        logW(LL_WARN, name, "- invalid size parameter", num, "for non-variable uniform");
        return;
      }
    }

    if constexpr (isCRGB) {
      if constexpr (isVecType) {
        // logQ("call to CRGB V", name, it->second, num);
        vector<Vector3> vec_col;
        unsigned int send_num = min(num, static_cast<int>(val.size()));
        vec_col.resize(send_num);
        for (unsigned int i = 0; i < send_num; ++i) {
          vec_col[i] = Vector3{static_cast<float>(val[i].r / 255.0f), static_cast<float>(val[i].g / 255.0f),
                               static_cast<float>(val[i].b / 255.0f)};
        }
        SetShaderValueV(shader, GetShaderLocation(shader, uf.c_str()), vec_col.data(), it->second, num);
      }
      else {
        Vector3 conv = {static_cast<float>(val.r / 255.0f), static_cast<float>(val.g / 255.0f),
                        static_cast<float>(val.b / 255.0f)};
        SetShaderValue(shader, GetShaderLocation(shader, uf.c_str()), &conv, it->second);
      }
    }
    else {
      if constexpr (isVecType) {
        // logQ("call to V", name, it->second, num);
        SetShaderValueV(shader, GetShaderLocation(shader, uf.c_str()), val.data(), it->second, num);
      }
      else {
        SetShaderValue(shader, GetShaderLocation(shader, uf.c_str()), &val, it->second);
      }
    }
  }

  void unloadData();

 private:
  string name;

  Shader shader;

  // map uniform name to input type
  map<string, int> typeMap;
};
