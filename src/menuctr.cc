#include "menuctr.h"
#include "enum.h"
#include "misc.h"

void menuController::registerMenu(menu& newMenu) {
  if (newMenu.type == TYPE_MAIN) {
    mainMenuOffset += newMenu.mainSize;
  }
  newMenu.render = false;
  menuSet.push_back(&newMenu);
}

void menuController::render() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    menuSet[i]->draw();
  }
}

void menuController::hide() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    menuSet[i]->render = false;
  }
}

void menuController::updateMouse() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    if (menuSet[i]->render == true) {
      if (pointInBox(getMousePosition(), (rect){menuSet[i]->x, menuSet[i]->y, menuSet[i]->width, menuSet[i]->height})) {
        mouseMenu = true;
        return;
      }
    }
  }
  mouseMenu = false;
}

bool menuController::mouseOnMenu() {
  return mouseMenu;
}

void menuController::updateRenderStatus() {
  for (const auto& i : menuSet) {
    if (i->parent) {
      if (!i->parent->render) {
        i->render = false;
      }
    }
  }
}

void menuController::unloadData() {
  for (const auto& i : menuSet) {
    if (i->type == TYPE_COLOR) {
      i->unloadData();
    }
  }
}
