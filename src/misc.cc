#include <algorithm>
#include "misc.h"
#include "data.h"
#include "log.h"

using std::min;
using std::max;

double getDistance(int x1, int y1, int x2, int y2) {
  double deltaX = abs(x1 - x2);
  double deltaY = abs(y1 - y2);
  return sqrt(pow(deltaX, 2) + pow(deltaY, 2));
}

colorHSV RGBtoHSV(const colorRGB& rgb) {
  colorHSV output;

  int r = rgb.r;
  int g = rgb.g;
  int b = rgb.b;

  double value = max({r, g, b});
  double minv = min({r, g, b});
  double chroma =  value - minv;
  double hue = 0;
  double saturation = chroma / value;

  // nonzero hue if nonzero chroma
  if (chroma != 0) {
    if (r == value) {
      // red is max
      hue = fmod(((g - b) / chroma), 6.0 ) * 60;
    }
    else if (g == value) {
      // green is max
      hue = (((b - r) / chroma) + 2) * 60;
    }
    else if (b == value) {
      // blue is min
      hue = (((r - g) / chroma) + 4) * 60;
    }
  }
  
  output.setHSV(hue, saturation, value);
  return output;
}

colorRGB HSVtoRGB(const colorHSV& hsv) {
  colorRGB output;
  double chroma = hsv.s * hsv.v;
  double m = hsv.v - chroma;
  double x = chroma * (1 - fabs(fmod((hsv.h / 60), 2) - 1));

  if (hsv.h >= 0 && hsv.h <= 60) {
    output.setRGB(chroma + m, x + m, m);
  }
  else if (hsv.h > 60 && hsv.h <= 120) {
    output.setRGB(x + m, chroma + m, m);
  }
  else if (hsv.h > 120 && hsv.h <= 180) {
    output.setRGB(m, chroma + m, x + m);
  }
  else if (hsv.h > 180 && hsv.h <= 240) {
    output.setRGB(m, x + m, chroma + m);
  }
  else if (hsv.h > 240 && hsv.h <= 300) {
    output.setRGB(x + m, m, chroma + m);
  }
  else if (hsv.h > 300 && hsv.h <= 360) {
    output.setRGB(chroma + m, m, x + m);
  }
  else {
    output.setRGB(m, m, m);
  }
  return output;
}

bool pointInBox(Vector2 mouse, rect box) {
  if (mouse.x >= box.x && mouse.x < box.x + box.width &&
      mouse.y >= box.y && mouse.y < box.y + box.height) {
    return true;
  }
  return false;
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

string getNoteInfo(int noteTrack, int notePos) {
  string result = "";
  int key = notePos % 12;
  int octave = (notePos + 9) / 12;
  switch(key) {
    case 0:
      result += "A";
      break;
    case 1:
      result += "A#/Bb";
      break;
    case 2:
      result += "B";
      break;
    case 3:
      result += "C";
      break;
    case 4:
      result += "C#/Db";
      break;
    case 5:
      result += "D";
      break;
    case 6:
      result += "D#/Eb";
      break;
    case 7:
      result += "E";
      break;
    case 8:
      result += "F";
      break;
    case 9:
      result += "F#/Gb";
      break;
    case 10:
      result += "G";
      break;
    case 11:
      result += "G#/Ab";
      break;
    default:
      return "";
      break;
  }
  result += to_string(octave);
  result += " | Track " + to_string(noteTrack + 1);
  return result;
}

string getSongPercent (double pos, double total) {
  string untruncText = to_string(100 * pos / total);
  double position = 100 * pos / total;

  string result = "100.00%";
  
  if (pos == 0) {
    return "0.00%";
  }
  if (untruncText[0] == 1) {
    result = untruncText.substr(0,6);
  }
  else if (pos == 0) {
    result = untruncText.substr(0,4);
  }
  else if (position > 0 && position < 10) {
    result = untruncText.substr(0,4);
  }
  else {
    result = untruncText.substr(0,5);
  }
  result += "%";
  return result;
}

string getSongTime(double pos, double total) {
  string result;
  
  result += toMinutes(floor(pos / 500.0));
  result += " / ";
  result += toMinutes(floor(total/500.0));
  return result;
}

string toMinutes(double seconds) {
  string result;

  int hours = seconds / 3600;
  int min = seconds / 60;
  int sec = static_cast<int>(round(seconds)) % 60;

  if (hours != 0) {
    string hrStr = to_string(hours);
    if (hours < 10) {
      hrStr = "0" + hrStr;
    }
    result += hrStr;
    result += ":";
  }
  
  string minStr = to_string(min);
  if (min < 10 && hours != 0) {
    minStr = "0" + minStr;
  }
  result += minStr;
  result += ":";
  
  string secStr = to_string(sec);
  if (sec < 10) {
    secStr = "0" + secStr;
  }
  result += secStr;

  return result;
}


