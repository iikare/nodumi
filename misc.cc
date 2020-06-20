#include <iostream>
#include <string>
#include <cmath>
#include "misc.h"

using std::cerr;
using std::endl;
using std::string;
using std::to_string;


bool hoverOnBox(int mouseX, int mouseY, int noteX, 
                 int noteY, int size) {
  return hoverOnBox(mouseX, mouseY, noteX, noteY, size, size);
}
bool hoverOnBox(int mouseX, int mouseY, int noteX, 
                 int noteY, int noteWidth, int noteHeight) {
  if (mouseX >= noteX && mouseX <= noteX + noteWidth &&
      mouseY >= noteY && mouseY <= noteY + noteHeight) {
    return true;
  }
  else {
    return false;
  }
}

void getMenuLocation(int mainW, int mainH, int cnX, int cnY, 
                     int& rcX, int& rcY, const int rcW, const int rcH) {
  // note: this function finds the menu starting point (X, Y) and stores them in
  // rcX / rxY 

  int mainXMin = MENU_MARGIN;
  int mainXMax = mainW - MENU_MARGIN;
  int mainYMin = MAIN_MENU_HEIGHT + MENU_MARGIN;
  int mainYMax = mainH - MENU_MARGIN;
  

  if (cnY + rcH > mainYMax) {
    // the menu would go off the bottom of the screen, set the bounds to end at the bottom margin
    rcY = mainYMax - rcH;
  }
  else if (cnY < mainYMin) {
    // menu starts in top margin space, set bound to top margin
    rcY = mainYMin;
  }
  else {
    // the menu can start at the note Y value
    rcY = cnY; 
  }

  if (cnX + rcW > mainXMax){
    // menu would go off the side of the screen, set it to end at right margin
    rcX = mainXMax - rcW;
  }
  else if (cnX < mainXMin) {
    // menu starts in left margin space, make it start at left margin
    rcX = mainXMin;
  }
  else{
    // the menu can start at the note Y value
    rcX = cnX;
  }
}

string getSongPercent (int pos, double total, bool end) {
  string untruncText = to_string(static_cast<double>(abs(100 * pos)/total));
  string result = "100.00%";
  // complete
  if (end) {
    return result;
  }
  if (untruncText[0] == 1) {
    result = untruncText.substr(0,6);
  }
  else if (pos == 0) {
    result = untruncText.substr(0,4);
  }
  else {
    result = untruncText.substr(0,5);
  }
  result += "%";
  return result;
}

void getColorSelectLocation(int mainW, int mainH, int& cpX, int& cpY, int rcX, int rcY, int xBuf, int yBuf) {
  cpX = rcX;
  cpY = rcY;

  int mainXMin = MENU_MARGIN;
  int mainXMax = mainW - MENU_MARGIN;
  int mainYMin = MAIN_MENU_HEIGHT + MENU_MARGIN;
  int mainYMax = mainH - MENU_MARGIN;
  
  int cX = cpX + xBuf/2;
  int cY = cpY + yBuf/2;
  int sX = COLOR_WIDTH + xBuf;
  int sY = COLOR_WIDTH + yBuf;

  cpX = cX - COLOR_WIDTH/2;
  cpY = cY - COLOR_HEIGHT/2;

  bool isXMax = cpX + sX > mainXMax;
  bool isXMin = cpX - sX < mainXMin;
  bool isYMax = cpY + sY > mainYMax;
  bool isYMin = cpY - sY < mainYMin;

  double ratio = static_cast<double>(xBuf)/yBuf;

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
    else if (cpX > mainW/2) {
      cpX -= sX;
    }
    else {
      cpX += sX;
    }

  }
  else if (isXMin && isYMin) {   
    // top left corner
    cpY += sX/ratio + yBuf;
    cpX += sX;
  }
  else if (isXMin && isYMax) {
    // bottom left corner
    cpY -= sX/ratio + yBuf;
    cpX += sX;
  }
  else if (isXMax && isYMin) {
    // top right corner
    cpY += sX/ratio + yBuf;
    cpX -= sX;
  }
  else if (isXMax && isYMax) {
    // bottom right corner
    cpY -= sX/ratio + yBuf;
    cpX -= sX;
  }
}

double getDistance(int x1, int y1, int x2, int y2) {
  double deltaX = abs(x1 - x2);
  double deltaY = abs(y1 - y2);
  return sqrt(pow(deltaX, 2) + pow(deltaY, 2));
}

colorRGB getHueByAngle(int x1, int y1, int x2, int y2) {
  // {x2, y2} iis the center point
  double deltaX = x1 - x2;
  double deltaY = y1 - y2;
  double angle = atan2(deltaY,deltaX) * 180.0/M_PI + 180;

  cerr << "angle: " << endl;
  colorHSV hsv ((angle/360.0)* 360, 1, 255);
  colorRGB result;
  result.setRGB(hsv);

  return result;
}
