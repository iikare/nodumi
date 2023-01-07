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

  // hacky test for graceful shader compilation failure
  if (shader.locs[5] == 3 && shader.locs[12] == 2) {
    logW(LL_WARN, "shader", item.assetName, "(id:"+ to_string(shader.id)+")", "failed to compile");
  }

  for (const auto& i : item.shaderUniforms) {
    typeMap.insert(make_pair(i.name, i.type));
  }

  name = item.assetName;
} 


void shaderData::unloadData() {
  UnloadShader(shader);
}
