#include "shadow.h"

#include "define.h"

void shadowController::init() { update(); }

void shadowController::update() {
  buffer = LoadRenderTexture(ctr.getWidth(), ctr.getHeight() - ctr.menuHeight);
}

void shadowController::unload() { UnloadRenderTexture(buffer); }
