#pragma once

#include <string>
#include <vector>
#include <raylib.h>
#include "misc.h"
#include "data.h"
#include "define.h"

using std::string;
using std::vector;

class menuController;

class menuItem {
  public:
    menuItem();
    
    string getContent();
    int getX();
    int getY();
    int getWidth();
    int getHeight();

    void setContent(string nContent);
    void setX(int nX);
    void setY(int nY);
    void setWidth(int nWidth);
    void setHeight(int nHeight);

  private:
    int x;
    int y;
    int width;
    int height;
    string content;
};

class menu {
  public:
    menu(point XY, vector<string> itemNames, menu* parentMenu, int menuType, int menuX = 0, int menuY = 0);

    int getX() { return x; }
    int getY() { return y; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getSize() { return itemCount; }
    int getItemX(int idx); 
    int getItemY(int idx); 
    int getActiveElement();
    string getContent(int idx);
    rect getBox(int idx);
    rect getSquare();
    colorRGB getColor();

    void setXY(int nX, int nY);
    void setContent(string nContent, int idx);
    void setActiveElement(int idx);
    void setSquare();
    void setAngle();
    void setColor(colorRGB col);
    
    void findActiveElement(point xy);

    void update(vector<string> itemNames);
    
    void addChildMenu(menu* child);
    void hideChildMenu();
    bool childOpen();
    bool parentOpen();

    bool clickCircle(int circleType);

    void draw();

    bool render;

    int mainSize;
    menu* parent;

    friend class menuController;
  private:
    int x;
    int y;
    int width;
    int height;
    int itemCount;
    int mainX;
    int mainY;
    int activeElement;
    int type;
    int pX = 0;
    int pY = 0;
    double angle = 0;
    vector<menuItem> items;
    vector<menu*> childMenu;
};

enum menuTypes {
  TYPE_MAIN,
  TYPE_SUB,
  TYPE_RIGHT,
  TYPE_RIGHTSUB,
  TYPE_COLOR
};
