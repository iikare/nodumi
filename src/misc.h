#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include <string>
#include <vector>
#include "color.h"
#include "note.h"
#include "box.h"

using std::vector;
using std::string;
using std::to_string;

double getFundamental(int y);
double fftAC(double f_1, double f_2);

double getDistance(int x1, int y1, int x2, int y2);

colorHSV RGBtoHSV(const colorRGB& rgb);
colorRGB HSVtoRGB(const colorHSV& hsv);

bool pointInBox(Vector2 pt, rect box);
bool pointInBox(point pt, rect box);
rect pointToRect(point a, point b);

string colorToHex(colorRGB col);

string getNoteInfo(int noteTrack, int notePos);

string getSongPercent (double pos, double total);
string getSongTime(double pos,  double total);
string toMinutes(double seconds);

vector<string>& formatPortName(vector<string>& ports);

bool isMKI(string path);
bool isValidPath(string path);

string toHex(int dec);
