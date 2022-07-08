#include <sys/stat.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "misc.h"
#include "data.h"
#include "log.h"

using std::min;
using std::max;
using std::stringstream;
using std::ifstream;
using std::hex;

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

bool pointInBox(Vector2 pt, rect box) {
  return pointInBox((point){static_cast<int>(pt.x), static_cast<int>(pt.y)}, box);
}
bool pointInBox(point pt, rect box) {
  if (pt.x >= box.x && pt.x < box.x + box.width &&
      pt.y >= box.y && pt.y < box.y + box.height) {
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

vector<int> getLinePositions(note* now, note* next) {
  vector<int> linePos = {};

  note* pNow = now;
  note* pNext = next;
  bool pushLine = false;

  if (!pNext) {
    if (!(pNow->isLastOnTrack)) {
      logW(LL_WARN, "pNext is nullptr");
    }
    return {};
  }

  auto pushVerts = [&] {
      linePos.push_back(now->number);
      
      linePos.push_back(pNow->x);
      linePos.push_back(pNow->y);
      
      if (!pushLine) {
        linePos.push_back(pNext->x);
        linePos.push_back(pNext->y);
      }
      else {
        linePos.push_back(pNow->x + pNow->duration);
        linePos.push_back(pNow->y);
      }
  };

  // only link spatially near notes
  if (now->x + 2 * now->duration < next->x) {
    pushLine = true;
  }


  if (now->getChordSize() == next->getChordSize()) {
    for (int i = 0; i < now->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
      pNext = pNext->chordNext;
    }
  }
  else if (now->getChordSize() > next->getChordSize()) {
    for (int i = 0; i < next->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
      if (i != next->getChordSize() - 1) {
        pNext = pNext->chordNext;
      }
    }
    for (int i = 0; i < now->getChordSize() - next->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
    }
  }
  else {
    for (int i = 0; i < now->getChordSize(); i++) {
      pushVerts();
      pNext = pNext->chordNext;
      if (i != now->getChordSize() - 1) {
        pNow = pNow->chordNext;
      }
    }
    for (int i = 0; i < next->getChordSize() - now->getChordSize(); i++) {
      pushVerts();
      pNext = pNext->chordNext;
    }
  }

  return linePos;
}

string colorToHex(colorRGB col) {
  stringstream result;
  result << "#"; 
  result << std::hex << std::setw(6) << std::setfill('0') << ((int)col.r << 16 | (int)col.g << 8 |(int) col.b << 0);
  string s = result.str(); 
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

rect pointToRect(point a, point b) {
  rect result = {0, 0, 0, 0};
  
  result.x = a.x - 2;
  result.y = min(a.y, b.y) - 4;
  result.width = b.x - a.x + 4;
  result.height = abs(b.y - a.y) + 8;

  return result;
}

vector<string>& formatPortName(vector<string>& ports) {

  for (unsigned int i = 0; i < ports.size(); i++) {
    int sp = 0;
    for (unsigned int j = 0; j < ports[i].length(); j++) {
      // break after second space/colon
      if (ports[i][j] == ' ' || ports[i][j] == ':') {
        sp++;
      }
      if (sp == 2) {
        ports[i] = ports[i].substr(0, j);
        break;
      }
    }
    ports[i].insert(0, to_string(i) + ": ");
  }

  return ports;
}

bool isMKI(string path) {


  transform(path.begin(), path.end(), path.begin(), ::tolower);
  string ext = path.substr(path.size() - min(3, static_cast<int>(path.length())));


  return ext == "mki";
}

bool isValidPath(string path) {
  struct stat info;
  // file doesn't exist in filesystem
  if (stat(path.c_str(), &info) == -1) {
    return false;
  }
  
  transform(path.begin(), path.end(), path.begin(), ::tolower);
  string ext = path.length() > 3 ? path.substr(path.size() - 3) : path;
  
  if (ext != "mid" && ext != "mki") {
    logW(LL_WARN, "invalid file extension:", path);
    logW(LL_WARN, "assuming default extension (mid)");
  }


  return true;
}



string toHex(int dec) {
  stringstream stream;
  stream << hex << dec;
  return stream.str();
}
