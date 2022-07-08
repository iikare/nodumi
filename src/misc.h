#pragma once

#include <string>
#include <vector>
#include <raylib.h>
#include "color.h"
#include "note.h"
#include "box.h"

using std::vector;
using std::string;
using std::to_string;

double getDistance(int x1, int y1, int x2, int y2);

colorHSV RGBtoHSV(const colorRGB& rgb);
colorRGB HSVtoRGB(const colorHSV& hsv);

bool pointInBox(Vector2 pt, rect box);
bool pointInBox(point pt, rect box);
rect pointToRect(point a, point b);


void getMenuLocation(int mainW, int mainH, int cnX, int cnY, int& rcX, int& rcY, const int rcW, const int rcH);
void getColorSelectLocation(int mainW, int mainH, int& cpX, int& cpY, int rcX, int rcY, int xBuf, int yBuf);

string colorToHex(colorRGB col);

string getNoteInfo(int noteTrack, int notePos);

string getSongPercent (double pos, double total);
string getSongTime(double pos,  double total);
string toMinutes(double seconds);

vector<string>& formatPortName(vector<string>& ports);

bool isMKI(string path);
bool isValidPath(string path);

string toHex(int dec);
