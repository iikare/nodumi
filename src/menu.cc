#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>
#include "enum.h"
#include "misc.h"
#include "menu.h"
#include "data.h"
#include "wrap.h"
#include "box.h"

using std::string;
using std::min;
using std::max;

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

menu::menu(point XY, vector<string> itemNames, int menuType, int menuX, int menuY, menu* parentMenu, int parentPos) :
           render(false), mainSize(ITEM_WIDTH), x(menuX), y(menuY), width(0), height(0),
           itemCount(itemNames.size()), mainX(XY.x), mainY(XY.y), activeElement(-1) {
  if (parentMenu != nullptr) {
    parentMenu->addChildMenu(this);
    parent = parentMenu;
  }
  else {
    parent = nullptr;
  }
  
  menuType != TYPE_COLOR ? width = ITEM_WIDTH : width = COLOR_WIDTH;
  menuType != TYPE_COLOR ? height = ITEM_HEIGHT * itemCount : height = COLOR_HEIGHT;
  items = {};
  childMenu = {};
  items.resize(itemCount);
  type = menuType;
  pX = 0;
  pY = 0;
  angle = 0;
  parentIndex = parentPos;

  for (int i = 0; i < itemCount; i++) {
    items[i].setContent(itemNames[i]);
    items[i].setX(x);
    items[i].setY(y + i * ITEM_HEIGHT);
    items[i].setWidth(ITEM_WIDTH);
    items[i].setHeight(ITEM_HEIGHT);
  }
  if (type == TYPE_MAIN) {
    mainSize = 8 + 1 + measureTextEx(itemNames[0].c_str()).x;
  }
  if (type == TYPE_COLOR) {
    // load square texture
    Image i = GenImageColor(squareDim, squareDim, WHITE);
    squareTex = LoadTextureFromImage(i);
    UnloadImage(i);
    
    i = GenImageColor(COLOR_WIDTH, COLOR_HEIGHT, WHITE);
    ringTex = LoadTextureFromImage(i);
    UnloadImage(i);
    
    ctr.setShaderValue("SH_RING", "ring_len", circleRatio); 
    ctr.setShaderValue("SH_RING", "ring_width", circleWidth); 
  }
}

int menu::getItemX(int idx) {
  if (idx >= itemCount || idx < 0) {
    logW(LL_WARN, "attempted to get menu itemX at nonexistent menu index ",idx);
    return -1;
  }
  return items[idx].getX();
}

int menu::getItemY(int idx) {
  if (idx >= itemCount || idx < 0) {
    logW(LL_WARN, "attempted to get menu itemY at nonexistent menu index",idx);
    return -1;
  }
  return items[idx].getY();
}

string menu::getContent(int idx) {
  if (idx >= itemCount || idx < 0) {
    logW(LL_WARN, "attempted to get menu content at nonexistent menu index", idx);
    return "";
  }
  return items[idx].getContent();
}

int menu::getActiveElement() {
  return activeElement;
}

void menu::setContent(string nContent, int idx) {
  if (idx >= itemCount || idx < 0) {
    logW(LL_WARN, "attempted to set menu item", nContent, "at menu index", idx);
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
    logW(LL_WARN, "attempted to set menu item at nonexistent menu index",idx);
    return;
  }
  activeElement = idx;
}

void menu::findActiveElement(point XY) {
  int curX = XY.x;
  int curY = XY.y;
  if (type == TYPE_MAIN) { 
        if (!pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)},
                       (rect){getItemX(0), getItemY(0), mainSize, ITEM_HEIGHT})){
          if (!pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)},
                         (rect){getItemX(1), getItemY(1), width, height - ITEM_HEIGHT})){
            activeElement = -1;
            return;
          }
        }
  }
  else if (type == TYPE_COLOR) {
    if (!pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)}, (rect){x, y, COLOR_WIDTH, COLOR_HEIGHT})) {
      activeElement = -1;
    }
    else {
      activeElement = 0;
    }
    return;
  }
  else { 
    if (!pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)}, (rect){x, y, width, height})) {
      activeElement = -1;
      return;
    }
  }
  for (int i = 0; i < itemCount; i++) {
    if (i == 0 && type == TYPE_MAIN) {
      if (pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)},
                     (rect){getItemX(i), getItemY(i), mainSize, ITEM_HEIGHT})){
      activeElement = i;
      break;
      } 
    }
    else {
      if (pointInBox((Vector2){static_cast<float>(curX), static_cast<float>(curY)},
                     (rect){getItemX(i), getItemY(i), width, ITEM_HEIGHT})){
        activeElement = i;
        break;
      }
    }
  }
}

void menu::update(vector<string> itemNames){
  
  activeElement = -1;
  itemCount = itemNames.size();
  height = ITEM_HEIGHT * itemCount;
  items.clear();
  items.resize(itemCount);
  
  for (int i = 0; i < itemCount; i++) {
    items[i].setContent(itemNames[i]);
    items[i].setX(x);
    items[i].setY(y + i * ITEM_HEIGHT);
    items[i].setWidth(ITEM_WIDTH);
    items[i].setHeight(ITEM_HEIGHT);
  }
}

void menu::unloadData() {
  if (type == TYPE_COLOR) {
    UnloadTexture(squareTex);
  }
}

rect menu::getBox(int idx) {
  rect result = {0, 0, 0, 0};
  if (idx == -1) {
    return result;
  }
  if (idx > itemCount || idx < 0) {
    logW(LL_WARN, "invalid call to getBox() with index", idx);
    return result;

  }
  
  result.x = getItemX(idx);
  result.y = getItemY(idx);
  result.width = getWidth();
  result.height = ITEM_HEIGHT;
  
  return result;
}

void menu::addChildMenu(menu* child) {
  childMenu.push_back(child);
}

void menu::hideChildMenu() {
  for (unsigned int i = 0; i < childMenu.size(); i++) {
    childMenu[i]->render = false;
  }
}

bool menu::childOpen() {
  for (unsigned int i = 0; i < childMenu.size(); i++) {
    if (childMenu[i]->render == true) {
      return true;
    }
  }
  return false;
}

bool menu::parentOpen() {
  if (parent != nullptr) {
    return parent->render;
  }
  return false;
}

vector<int> menu::findOpenChildMenu() {
  vector<int> result;

  for (unsigned int i = 0; i < childMenu.size(); i++) {
    if (childMenu[i]->render) {
      result.push_back(childMenu[i]->parentIndex);
      //logQ("child index is", childMenu[i]->parentIndex);
    }
  }

  return result;
}


void menu::setSquare() {

  const float circleX = x + COLOR_WIDTH/2.0f;
  const float circleY = y + COLOR_HEIGHT/2.0f;
  
  if (type == TYPE_COLOR) {
    pX = round(max(min(ctr.getMousePosition().x - circleX + squareDim/2.0, double(squareDim)), 0.0));
    pY = round(max(min(ctr.getMousePosition().y - circleY + squareDim/2.0, double(squareDim)), 0.0));
  }
}

rect menu::getSquare() {
  const float circleX = x + COLOR_WIDTH/2.0f;
  const float circleY = y + COLOR_HEIGHT/2.0f;

  rect result = {0, 0, 0, 0};
  result.x = circleX - squareDim/2.0;
  result.y = circleY - squareDim/2.0;
  result.width = squareDim;
  result.height = squareDim;
  
  return result;
}

void menu::setAngle() {
 
  const float circleX = x + COLOR_WIDTH/2.0f;
  const float circleY = y + COLOR_HEIGHT/2.0f;

  if (type == TYPE_COLOR) {
    int deltaX = circleX - ctr.getMouseX();
    int deltaY = circleY - ctr.getMouseY();
    angle = atan2(-deltaY, deltaX) * 180.0/M_PI + 180;
    angle = fmod(angle, 360.0);

    Color a = ColorFromHSV(angle,1,1);//{244.0f/255.0f,0.0f/255.0f,244.0f/255.0f};
    Vector3 col = {static_cast<float>(a.r/255.0f), static_cast<float>(a.g/255.0f), static_cast<float>(a.b/255.0f)};
    ctr.setShaderValue("SH_SQUARE", "blend_color", col);
  }
}

bool menu::clickCircle(int circleType) {
  
  const float circleRatio = 0.425;
  const float circleWidth = 0.075;
  const float squareDim = (circleRatio - circleWidth - 0.05) * COLOR_WIDTH * sqrt(2);
  const float circleX = x + COLOR_WIDTH/2.0f;
  const float circleY = y + COLOR_HEIGHT/2.0f;

  if (circleType == 0) {
    if (getDistance(ctr.getMouseX(), ctr.getMouseY(), circleX, circleY) > (circleRatio - circleWidth) * COLOR_WIDTH) {
      return true;
    }
  }
  if (circleType == 1) {
    if (getDistance(ctr.getMouseX(), ctr.getMouseY(), circleX - squareDim/2.0 + pX, circleY - squareDim/2.0 + pY) < 5.0f) {
        return true;
    }
  }
  return false;

}
    
void menu::setColor(colorRGB col) {
  
  if (col.r == 0 && col.g == 0 && col.b == 0) {
    angle = 0;
    //pX = 0;
    pY = squareDim + 1;
    return;
  }

  colorHSV colHSV = RGBtoHSV(col);
  
  angle = colHSV.h;
  pX = round(colHSV.s * squareDim);
  pY = (1.0 - min(1.0, colHSV.v/255.0)) * (squareDim + 1);

  Color a = ColorFromHSV(angle,1,1);//{244.0f/255.0f,0.0f/255.0f,244.0f/255.0f};
  Vector3 colV = {static_cast<float>(a.r/255.0f), static_cast<float>(a.g/255.0f), static_cast<float>(a.b/255.0f)};
  ctr.setShaderValue("SH_SQUARE", "blend_color", colV);
  
}

colorRGB menu::getColor() {
  
  colorHSV col;
  
  col.h = angle;
  col.s = (pX/squareDim);
  col.v = round((255 - (pY)/squareDim * 255));

  return HSVtoRGB(col);
}

void menu::draw() {
  findActiveElement(ctr.getMousePosition());
  if (type == TYPE_MAIN) {
    if (render) {
      drawRectangle(x, y, mainSize, ITEM_HEIGHT, ctr.bgMenuShade);
      drawRectangle(x, getItemY(1), width, height - ITEM_HEIGHT, ctr.bgMenu);
      if (getActiveElement() != -1) {
        if (getActiveElement() == 0) {
          drawRectangle(getItemX(getActiveElement()), getItemY(getActiveElement()), mainSize, ITEM_HEIGHT, ctr.bgMenuShade);
        }
        else {
          drawRectangle(getItemX(getActiveElement()), getItemY(getActiveElement()), ITEM_WIDTH, ITEM_HEIGHT, ctr.bgMenuShade);
        }
      }

      
      vector<int> openChildren = findOpenChildMenu();

      for (unsigned int i = 0; i < openChildren.size(); i++) {
        drawRectangle(getItemX(openChildren[i]), getItemY(openChildren[i]), ITEM_WIDTH, ITEM_HEIGHT, ctr.bgMenuShade);
      }
      
      for (int i = 1; i < itemCount; i++) {
        drawLineEx(x, getItemY(i) + 1, x + ITEM_WIDTH, getItemY(i) + 1, 0.5, ctr.bgMenuLine);
      }
      for (int i = 0; i < itemCount; i++) {
        if (i == 0) {
          drawTextEx(getContent(i).c_str(), getItemX(i) + 4, getItemY(i) + 4, ctr.bgDark);
        }
        else {
          drawTextEx(getContent(i).c_str(), getItemX(i) + 4, getItemY(i) + 5, ctr.bgDark);
        }
      }
    }
    else { 
      if (getActiveElement() == 0) {
        drawRectangle(getItemX(getActiveElement()), getItemY(getActiveElement()), mainSize, ITEM_HEIGHT, ctr.bgMenuShade);
      }
      else {
        drawRectangle(x, y, mainSize, ITEM_HEIGHT, ctr.bgMenu);
      }
      drawTextEx(getContent(0).c_str(), getItemX(0) + 4, getItemY(0) + 4, ctr.bgDark);
    }  
  }
  else if (type == TYPE_COLOR) {
    if (render) {
      
      const float circleX = x + COLOR_WIDTH/2.0f;
      const float circleY = y + COLOR_HEIGHT/2.0f;

      drawRectangle(x, y, COLOR_WIDTH, COLOR_HEIGHT, ctr.bgMenu);
      
      ctr.beginShaderMode("SH_RING");
        drawTextureEx(ringTex, {static_cast<float>(x), static_cast<float>(y)});
      ctr.endShaderMode();
      
      ctr.beginShaderMode("SH_SQUARE");
        drawTextureEx(squareTex, {circleX-squareDim/2.0f, circleY-squareDim/2.0f});
      ctr.endShaderMode();


      drawRing({float(circleX - squareDim/2.0 + pX), float(circleY - squareDim/2.0 + pY)}, 
               0.0f, 5.0f, ColorFromHSV(float(fmod(angle, 360.0)), 0.3f, 1.0f)); 
      
      drawRing({float(circleX + (circleRatio - circleWidth/2.0) * COLOR_WIDTH * cos(angle * M_PI/180.0)),
                float(circleY - (circleRatio - circleWidth/2.0) * COLOR_HEIGHT * sin(angle * M_PI/180.0))},
                0.0f, 5.0f, ColorFromHSV(float(fmod(angle, 360.0)), 0.3f, 1.0f)); 
      
      drawRectangle(x + COLOR_WIDTH - 36, y + COLOR_HEIGHT - 36, 36, 36, getColor());

      int yOffset = 1 + measureTextEx(colorToHex(getColor()).c_str()).y;
      drawTextEx(colorToHex(getColor()).c_str(), x + 4, y + COLOR_HEIGHT - yOffset - 2, ctr.bgDark);
    }
  }
  else {
    if (render) {
      drawRectangle(x, y, width, height, ctr.bgMenu);
      if (getActiveElement() != -1) {
        drawRectangle(getItemX(getActiveElement()), getItemY(getActiveElement()), ITEM_WIDTH, ITEM_HEIGHT, ctr.bgMenuShade);
      }
      
      bool rightFlag = false;
      if (type == TYPE_RIGHT) {
        rightFlag = true;
      }

      bool lineFlag = false;
      if (parent != nullptr && parent->getItemY(1) < getItemY(0)) {
        lineFlag = true;
      }

      if (!rightFlag) {
        drawLineEx(x, y, x, y + height, 0.5, ctr.bgMenuLine);
      }

      for (int i = 0; i < itemCount; i++) {
        if (i == 0 && lineFlag) {
          drawLineEx(x, getItemY(i) + 1, x + ITEM_WIDTH, getItemY(i) + 1, 1, {0, 0, 0});
        }
        else if (!rightFlag) {
          drawLineEx(x, getItemY(i) + 1, x + ITEM_WIDTH, getItemY(i) + 1, 0.5, ctr.bgMenuLine);
        }
        rightFlag = false;
      }

      for (int i = 0; i < itemCount; i++) {
        drawTextEx(getContent(i).c_str(), getItemX(i) + 4, getItemY(i) + 5, ctr.bgDark);
      }
    }
  }
}

void menu::findMenuLocation(int& rcX, int& rcY) {

  // note: this function finds the menu starting point (X, Y) and stores them in
  // rcX / rxY 

  int mainXMin = MENU_MARGIN;
  int mainXMax = ctr.getWidth() - MENU_MARGIN;
  int mainYMin = MAIN_MENU_HEIGHT + MENU_MARGIN;
  int mainYMax = ctr.getHeight() - MENU_MARGIN;
  

  if (ctr.getMouseY() + getHeight() > mainYMax) {
    // the menu would go off the bottom of the screen, set the bounds to end at the bottom margin
    rcY = mainYMax - getHeight();
  }
  else if (ctr.getMouseY() < mainYMin) {
    // menu starts in top margin space, set bound to top margin
    rcY = mainYMin;
  }
  else {
    // the menu can start at the note Y value
    rcY = ctr.getMouseY(); 
  }

  if (ctr.getMouseX() + getWidth() > mainXMax){
    // menu would go off the side of the screen, set it to end at right margin
    rcX = mainXMax - getWidth();
  }
  else if (ctr.getMouseX() < mainXMin) {
    // menu starts in left margin space, make it start at left margin
    rcX = mainXMin;
  }
  else{
    // the menu can start at the note Y value
    rcX = ctr.getMouseX();
  }
}
void menu::findColorSelectLocation(int& cpX, int& cpY, int rcX, int rcY) {

  cpX = rcX;
  cpY = rcY;

  int mainXMin = MENU_MARGIN;
  int mainXMax = ctr.getWidth() - MENU_MARGIN;
  int mainYMin = MAIN_MENU_HEIGHT + MENU_MARGIN;
  int mainYMax = ctr.getHeight() - MENU_MARGIN;
  
  int cX = cpX + width/2;
  int cY = cpY + height/2;
  int sX = COLOR_WIDTH + width;
  int sY = COLOR_WIDTH + height;

  cpX = cX - COLOR_WIDTH/2;
  cpY = cY - COLOR_HEIGHT/2;

  bool isXMax = cpX + sX > mainXMax;
  bool isXMin = cpX - sX < mainXMin;
  bool isYMax = cpY + sY > mainYMax;
  bool isYMin = cpY - sY < mainYMin;

  double ratio = static_cast<double>(width)/height;

  if (isXMax + isXMin + isYMax + isYMin <= 1) {
    if (isXMax) {
      cpX -= sX;
    }
    else if (isXMin) {
      cpX += sX;
    }

    if (isYMax) {
      cpY -= sY;
    }
    else if (isYMin) {
      cpY += sY;
    }
    else if (cpX > ctr.getWidth()/2) {
      cpX -= sX;
    }
    else {
      cpX += sX;
    }

  }
  else if (isXMin && isYMin) {   
    // top left corner
    cpY += sX/ratio + height;
    cpX += sX;
  }
  else if (isXMin && isYMax) {
    // bottom left corner
    cpY -= sX/ratio + height;
    cpX += sX;
  }
  else if (isXMax && isYMin) {
    // top right corner
    cpY += sX/ratio + height;
    cpX -= sX;
  }
  else if (isXMax && isYMax) {
    // bottom right corner
    cpY -= sX/ratio + height;
    cpX -= sX;
  }
}
