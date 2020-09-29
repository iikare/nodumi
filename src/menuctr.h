#pragma once

#include <vector>
#include <raylib.h>
#include "menu.h" 
#include "log.h"

using std::vector;

class menuController {
  public:
    menuController() {
      mainMenuOffset = 0;
      mouseMenu = false;
      menuSet = {};
    }
    

    void registerMenu(menu* newMenu);
    void renderAll();
    void hideAll();

    bool mouseOnMenu();

    void updateMouse();
    
    int getOffset() { return mainMenuOffset; }
  private:
    int mainMenuOffset;
    bool mouseMenu;
    vector<menu*> menuSet;
    
};
