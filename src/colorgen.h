#pragma once

#include <vector>

#include "color.h"
#include "enum.h"
#include "kmeans.h"

using std::pair;
using std::vector;

void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>& on, vector<colorRGB>& off);

void cycleColorScheme(vector<colorRGB>& on, vector<colorRGB>& off);

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                    const vector<pair<int, double>>& weight = {});

void getColorSchemeImage(schemeType type, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         const vector<pair<int, double>>& weight = {});
void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         const vector<pair<int, double>>& weight);

vector<colorRGB> findKMeans(vector<kMeansPoint>& colorData, int k);
