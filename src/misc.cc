#include <sys/stat.h>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "misc.h"
#include "data.h"
#include "log.h"

using std::min;
using std::max;
using std::stringstream;
using std::hex;

double getFundamental(int y) {
  return pow(2, (y-69)/12.0) * 440;
}

// f_1 is the original frequency, f_2 is the correlation frequency
double fftAC(double f_1, double f_2) {
  // func: sin(f_1) * sin(f_2)
  
  double sum = 0;
  for (auto i = 0; i < FFT_AC_BINS; ++i) {
    double fN = sin(f_1) * sin(f_2);
    double fNpK = sin(f_1+i) * sin(f_2+i);
    sum+=log(1+fabs(fN*fNpK / ((f_1-f_2)/f_1)));
  }

  return fabs(sum/FFT_AC_BINS);
}

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
  return pt.x >= box.x && pt.x < box.x + box.width &&
         pt.y >= box.y && pt.y < box.y + box.height;
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
    //logW(LL_WARN, "invalid file extension:", path);
    //logW(LL_WARN, "assuming default extension (mid)");
    return false;
  }


  return true;
}



string toHex(int dec) {
  stringstream stream;
  stream << hex << dec;
  return stream.str();
}
