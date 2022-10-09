#include <type_traits>
#include "shader.h"
#include "enum.h"
#include "log.h"

using std::make_pair;
using std::is_same;

shaderData::shaderData(const asset& item) {
  if (item.assetType != ASSET_SHADER) {
    logW(LL_WARN, "attempt to load non-shader of type:", item.assetType);
    return;
  }

  shader = LoadShaderFromMemory(nullptr, reinterpret_cast<char*>(const_cast<unsigned char*>(item.data)));

  for (const auto& i : item.shaderUniforms) {
    int loc = GetShaderLocation(shader, i.name.c_str());
    typeMap.insert(make_pair(i.name, make_pair(loc,i.type)));
  }

  name = item.assetName;
} 

template <class T>
void shaderData::setShaderValue(string& uf, T& val) {

  // verify types
  static_assert(is_same<T, Vector2>::value || 
                is_same<T, Vector3>::value || 
                is_same<T, float>::value, 
                "invalid type passed");
      

  auto it = typeMap.find(uf);

  if (it == typeMap.end()) {
    logW(LL_WARN, name, "- attempt to modify non-existent uniform:", uf);
    return;
  }


  SetShaderValue(shader, it->second.first, &val, it->second.second);
}

void shaderData::unloadData() {
  UnloadShader(shader);
}
