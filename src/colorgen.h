#pragma once

#include <vector>
#include "color.h"

using std::vector;
using std::pair;

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off);
