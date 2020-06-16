#include <iostream>
#include "misc.h"
#include "menu.h"

using std::cerr;
using std::endl;

menu::menu() {
  cerr << "error: class menu can NOT be initialized without the proper constructor!" << endl;
  exit(1);
}

menu::menu(int winX, int winY, int itemCount) : width(0), height(0), itemCount(itemCount),
                                               mainX(winX), mainY(winY), items(nullptr) {
  height = ITEM_HEIGHT * itemCount;
  width = ITEM_WIDTH;
  items = new menuItem[itemCount];

}

menu::~menu() {
  delete[] items;
}
