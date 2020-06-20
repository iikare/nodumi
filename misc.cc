#include <iostream>
#include <string>
#include "misc.h"

using std::cerr;
using std::endl;
using std::string;
using std::to_string;


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

void getColorSelectLocation(int mainW, int mainH, int& rcX, int& rcY, int xBuf, int yBuf) {

  int mainXMin = MENU_MARGIN;
  int mainXMax = mainW - MENU_MARGIN;
  int mainYMin = MAIN_MENU_HEIGHT + MENU_MARGIN;
  int mainYMax = mainH - MENU_MARGIN;
  
  int cX = rcX + xBuf/2;
  int cY = rcY + yBuf/2;
  int sX = COLOR_WIDTH + xBuf;
  int sY = COLOR_WIDTH + yBuf;

  rcX = cX - COLOR_WIDTH/2;
  rcY = cY - COLOR_HEIGHT/2;

  bool isXMax = rcX + sX > mainXMax;
  bool isXMin = rcX - sX < mainXMin;
  bool isYMax = rcY + sY > mainYMax;
  bool isYMin = rcY - sY < mainYMin;

  double ratio = static_cast<double>(xBuf)/yBuf;

  if (isXMax + isXMin + isYMax + isYMin <= 1) {
    if (isXMax) {
      rcX -= sX;
    }
    else if (isXMin) {
      rcX += sX;
    }

    if (isYMax) {
      rcY -= sY;
    }
    else if (isYMin) {
      rcY += sY;
    }
    else if (rcX > mainW/2) {
      rcX -= sX;
    }
    else {
      rcX += sX;
    }

  }
  else if (isXMin && isYMin) {   
    // top left corner
    rcY += sX/ratio + yBuf;
    rcX += sX;
  }
  else if (isXMin && isYMax) {
    // bottom left corner
    rcY -= sX/ratio + yBuf;
    rcX += sX;
  }
  else if (isXMax && isYMin) {
    // top right corner
    rcY += sX/ratio + yBuf;
    rcX -= sX;
  }
}
