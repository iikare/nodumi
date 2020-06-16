#ifndef MENU_H
#define MENU_H

class menuItem {
  public:
  private:
};

class menu {
  public:
    menu();
    menu(int winX, int winY, int itemCount);
    ~menu();

  private:
    int width;
    int height;
    int itemCount;
    int mainX;
    int mainY;
    menuItem* items;
};




#endif
