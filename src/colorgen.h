#pragma once

#include <vector>
#include "color.h"
#include "kmeans.h"

using std::vector;
using std::pair;


void invertColorScheme(colorRGB& bg, colorRGB& line, vector<colorRGB>* on, vector<colorRGB>* off);

void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorScheme(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB);
void getColorSchemeImage(int n, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB,
                         vector<pair<int, double>>& weight);
void getColorSchemeImage(int n, int k, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, vector<pair<int, double>>& weight);

vector<colorRGB> findKMeans(vector<kMeansPoint>& colorData, int k, int& meanV);

