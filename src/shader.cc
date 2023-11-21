#include <type_traits>
#include <cstring>
#include "shader.h"
#include "enum.h"
#include "log.h"

using std::make_pair;

shaderData::shaderData(const asset& item) {
  if (item.assetType != ASSET::SHADER) {
    logW(LL_WARN, "attempt to load non-shader of type:", item.assetType);
    return;
  }

  // strcmp != 0 -> not equal
  bool vs = strcmp(reinterpret_cast<char*>(item.dataVS),"") != 0;
  bool fs = strcmp(reinterpret_cast<char*>(item.dataFS),"") != 0;

  if (vs && fs) {
    shader = LoadShaderFromMemory(reinterpret_cast<char*>(const_cast<unsigned char*>(item.dataVS)),
                                  reinterpret_cast<char*>(const_cast<unsigned char*>(item.dataFS)));
  }
  else if (vs) {
    shader = LoadShaderFromMemory(reinterpret_cast<char*>(const_cast<unsigned char*>(item.dataVS)), nullptr);
  }
  else if (fs) {
    shader = LoadShaderFromMemory(nullptr, reinterpret_cast<char*>(const_cast<unsigned char*>(item.dataFS)));
  }
  else {
    logW(LL_WARN, "attempt to load shader without valid shader data:", item.assetName);
    return;
  }

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
