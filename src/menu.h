#pragma once

#include <string>
#include <vector>

#include "build_target.h"
#include "data.h"
#include "enum.h"
#include "misc.h"

using std::string;
using std::vector;

class menuController;

class menuItem {
 public:
  menuItem();

  string getContent() const;
  int getX() const;
  int getY() const;
  int getWidth() const;
  int getHeight() const;

  void setContent(const string& nContent);
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
  menu(point XY, menuContentType contentType, int menuType, int menuX = 0,
       int menuY = 0, menu* parentMenu = nullptr, int parentPos = -1);

  int getX() const { return x; }
  int getY() const { return y; }
  int getWidth() const { return width; }
  int getHeight() const { return height; }
  int getSize() const { return itemCount; }
  int getItemX(int idx) const;
  int getItemY(int idx) const;
  int getActiveElement() const;
  string getContent(int idx) const;
  rect getBox(int idx) const;
  rect getSquare() const;
  colorRGB getColor() const;

  bool isContentLabel(const string& label, int idx) const;
  bool isActive() const { return getActiveElement() == MENU_INACTIVE; }

  void setXY(int nX, int nY);
  void setContent(const string& nContent, int idx);
  void setContentLabel(const string& label, int idx);
  void setActiveElement(int idx);
  void setSquare();
  void setAngle();
  void setColor(const colorRGB& col);

  void swapLabel(const string& str_id1, const string& str_id2, int idx);

  void findActiveElement(point xy);

  void findMenuLocation(int& rcX, int& rcY) const;
  void findColorSelectLocation(int& cpX, int& cpY, int rcX, int rcY) const;

  void update(const vector<string>& itemNames);
  void unloadData();

  void addChildMenu(menu* child);
  void hideChildMenu();
  bool childOpen() const;
  bool parentOpen() const;

  vector<int> findOpenChildMenu() const;

  bool clickCircle(int circleType) const;

  void draw();

  bool render;

  int mainSize;
  menu* parent;
  int parentIndex;

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

  Texture2D squareTex;
  Texture2D ringTex;

  void setRingColor();

  static constexpr float circleRatio = 0.425;
  static constexpr float circleWidth = 0.075;
  static constexpr float squareDim =
      (circleRatio - circleWidth - 0.05) * COLOR_WIDTH * 1.414;
};
