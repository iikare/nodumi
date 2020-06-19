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
