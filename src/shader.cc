#include <type_traits>
#include "shader.h"
#include "enum.h"
#include "log.h"

using std::make_pair;

shaderData::shaderData(const asset& item) {
  if (item.assetType != ASSET_SHADER) {
    logW(LL_WARN, "attempt to load non-shader of type:", item.assetType);
    return;
  }

  shader = LoadShaderFromMemory(nullptr, reinterpret_cast<char*>(const_cast<unsigned char*>(item.data)));

  for (const auto& i : item.shaderUniforms) {
    typeMap.insert(make_pair(i.name, i.type));
  }

  name = item.assetName;
} 


void shaderData::unloadData() {
  UnloadShader(shader);
}
