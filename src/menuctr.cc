#include "menuctr.h"
#include "misc.h"

void menuController::registerMenu(menu* newMenu) {
  if (newMenu->type == TYPE_MAIN) {
    mainMenuOffset += newMenu->mainSize;
  }
  newMenu->render = false;
  menuSet.push_back(newMenu);
}

void menuController::renderAll() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    menuSet[i]->draw();
  }
}

void menuController::hideAll() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    menuSet[i]->render = false;
  }
}

void menuController::updateMouse() {
  for (unsigned int i = 0; i < menuSet.size(); i++) {
    if (menuSet[i]->render == true) {
      if (pointInBox(GetMousePosition(), (rect){menuSet[i]->x, menuSet[i]->y, menuSet[i]->width, menuSet[i]->height})) {
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
