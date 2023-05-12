#include <sys/stat.h>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "misc.h"
#include "data.h"
#include "wrap.h"
#include "define.h"
#include "log.h"

using std::min;
using std::to_string;
using std::stringstream;
using std::hex;

double getDistance(int x1, int y1, int x2, int y2) {
  double deltaX = abs(x1 - x2);
  double deltaY = abs(y1 - y2);
  return sqrt(pow(deltaX, 2) + pow(deltaY, 2));
}

string getNoteInfo(int noteTrack, int notePos, bool isFlat) {
  string result = "";
  int key = notePos % 12;
  int octave = (notePos + 9) / 12;
  switch(key) {
    case 0:
      result += "A";
      break;
    case 1:
      result += isFlat ? "Bb" : "A#";
      break;
    case 2:
      result += "B";
      break;
    case 3:
      result += "C";
      break;
    case 4:
      result += isFlat ? "Db" : "C#";
      break;
    case 5:
      result += "D";
      break;
    case 6:
      result += isFlat ? "Eb" : "D#";
      break;
    case 7:
      result += "E";
      break;
    case 8:
      result += "F";
      break;
    case 9:
      result += isFlat ? "Gb" : "F#";
      break;
    case 10:
      result += "G";
      break;
    case 11:
      result += isFlat ? "Ab" :"G#";
      break;
    default:
      return "";
      break;
  }
  result += to_string(octave);
  result += " | " + ctr.text.getString("GET_SONG_INFO_TRACK") + " " + to_string(noteTrack + 1);
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

string colorToHex(const colorRGB& col) {
  stringstream result;
  result << "#"; 
  result << std::hex << std::setw(6) << std::setfill('0') << ((int)col.r << 16 | (int)col.g << 8 |(int) col.b << 0);
  string s = result.str(); 
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

rect pointToRect(const point& a, const point& b) {
  rect result = {0, 0, 0, 0};
  
  result.x = a.x - 2;
  result.y = min(a.y, b.y) - 4;
  result.width = b.x - a.x + 4;
  result.height = abs(b.y - a.y) + 8;

  return result;
}

point getMousePosition() { 
  if (IsWindowFocused()) {
    return point(GetMouseX(), GetMouseY()); 
  }
  return {-1,-1};
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

string getExtension(const string& path, bool len4) {
  unsigned long len = len4 ? 4 : 3;
  string p = path;
  transform(p.begin(), p.end(), p.begin(), ::tolower);
  return p.length() > len ? p.substr(p.size() - len) : p;
}

string getDirectory(const string& path) {
  string dir = path;
  int pos = dir.find_last_of("/\\");
  return dir.substr(0,pos);
}

bool isValidPath(const string& path, int pathTypes...) {
  struct stat info;
  // file doesn't exist in filesystem
  if (stat(path.c_str(), &info) == -1) {
    return false;
  }
  
  string ext = getExtension(path);

  return isValidExtension(ext, pathTypes);

}

bool isValidExtension(const string& ext, int pathType) {
  switch (pathType) {
    case PATH_DATA:
      return ext == "mid" || ext == "mki";
    case PATH_IMAGE:
      return ext == "png" || ext == "jpg";
    case PATH_MKI:
      return ext == "mki";
    default:
      return false;
  }
}

bool isValidExtension(const string& ext, int pathType, int pathTypes...) {
  return isValidExtension(ext, pathType) || isValidExtension(ext, pathTypes);
}

string toHex(int dec) {
  stringstream stream;
  stream << hex << dec;
  return stream.str();
}

colorRGB maximizeDeltaE(const colorRGB& ref) {
  // resticted to grayscale only
  // v \in [0,255]
  colorLAB r1(ref);
  unsigned int maxDE = 0;
  unsigned char optV = 0;
  for (unsigned int v = 0; v <= 255; ++v) {
    colorLAB r2(colorRGB(v,v,v));
    double dE = deltaE(r1, r2);
    if (dE > maxDE) {
      maxDE = dE;
      optV = v;
      //logQ(v,":", dE);
    }
  }
  //logQ(optV,":", maxDE);
  return colorRGB(optV, optV, optV);
  //return ctr.bgLight;
}
