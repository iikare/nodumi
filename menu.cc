#include <iostream>
#include <string>
#include <vector>
#include "misc.h"
#include "menu.h"


using std::cerr;
using std::endl;
using std::string;


menuItem::menuItem() : x(0), y(0), width(0), height(0), content("") {}

string menuItem::getContent() {
  return content;
}

int menuItem::getX() {
  return x;
}

int menuItem::getY() {
  return y;
}

int menuItem::getWidth() {
  return width;
}

int menuItem::getHeight() {
  return height;
}

void menuItem::setContent(string nContent) {
  content = nContent;
}

void menuItem::setX(int nX) {
  x = nX;
}

void menuItem::setY(int nY) {
  y = nY;
}

void menuItem::setWidth(int nWidth) {
  width = nWidth;
}

void menuItem::setHeight(int nHeight) {
  height = nHeight;
}



menu::menu() {
  cerr << "error: menu can NOT be initialized without the proper constructor!" << endl;
  exit(1);
}

menu::menu(int winX, int winY, vector<string> itemNames, bool mainMenu, int menuX, int menuY) :
           render(false), isMainMenu(mainMenu), x(menuX), y(menuY), width(0), height(0),
           itemCount(itemNames.size()), mainX(winX), mainY(winY), activeElement(-1),
           items(nullptr) {

  height = ITEM_HEIGHT * itemCount;
  width = ITEM_WIDTH;
  items = new menuItem[itemCount];
  for (int i = 0; i < itemCount; i++) {
    items[i].setContent(itemNames[i]);
    items[i].setX(x);
    items[i].setY(y + i * ITEM_HEIGHT);
    items[i].setWidth(ITEM_WIDTH);
    items[i].setHeight(ITEM_HEIGHT);
  }
}

menu::~menu() {
  delete[] items;
}

int menu::getItemX(int idx) {
  if (idx >= itemCount || idx < 0) {
    cerr << "warn: attempted to get menu itemX at nonexistent menu index " << idx << endl;
    return -1;
  }
  return items[idx].getX();
}

int menu::getItemY(int idx) {
  if (idx >= itemCount || idx < 0) {
    cerr << "warn: attempted to get menu itemY at nonexistent menu index " << idx << endl;
    return -1;
  }
  return items[idx].getY();
}

string menu::getContent(int idx) {
  if (idx >= itemCount || idx < 0) {
    cerr << "warn: attempted to get menu item at nonexistent menu index " << idx << endl;
    return "";
  }
  return items[idx].getContent();
}

int menu::getActiveElement() {
  return activeElement;
}

void menu::setContent(string nContent, int idx) {
  if (idx >= itemCount || idx < 0) {
    cerr << "warn: attempted to set menu item " << nContent << " at menu index " << idx << endl;
    return;
  }
  items[idx].setContent(nContent);
}

void menu::setXY(int nX, int nY) {
  x = nX;
  y = nY; 
  for (int i = 0; i < itemCount; i++) {
    items[i].setX(x);
    items[i].setY(y + i * ITEM_HEIGHT);
  }
}

void menu::setActiveElement(int idx) {
  if (idx >= itemCount || idx < 0) {
    cerr << "warn: attempted to reference menu element at nonexistent menu index " << idx << endl;
    return;
  }
  activeElement = idx;
}

void menu::findActiveElement(int curX, int curY) {
  if (!hoverOnBox(curX, curY, x, y, width, height)) {
    activeElement = -1;
    return;
  }
  else {
    for (int i = 0; i < itemCount; i++) {
      if (hoverOnBox(curX, curY, getItemX(i), getItemY(i), width, height/itemCount)){
        activeElement = i;
        break;
      }
    }
  }
}

  

