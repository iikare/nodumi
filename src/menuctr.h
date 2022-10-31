#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include <vector>
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
    

    void registerMenu(menu& newMenu);
    void render();
    void hide();

    void unloadData();

    bool mouseOnMenu();

    void updateMouse();
    void updateRenderStatus();
    
    int getOffset() { return mainMenuOffset; }
  private:
    int mainMenuOffset;
    bool mouseMenu;
    vector<menu*> menuSet;
    
};
