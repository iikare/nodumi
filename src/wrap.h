#pragma once

#include <string>
#include <raylib.h>
#include "define.h"
#include "color.h"
#include "data.h"
#include "../dpd/osdialog/osdialog.h"

using std::string;

void clearBackground(colorRGB col);
void drawRectangle(float x, float y, float w, float h, colorRGB col);
void drawLine(int xi, int yi, int xf, int yf, colorRGB col);
void drawLineEx(int xi, int yi, int xf, int yf, float thick, colorRGB col);
void drawTextEx(Font ft, string msg, Vector2 pos, colorRGB col, unsigned char alpha = 255);
void drawTextEx(Font ft, string msg, int x, int y, colorRGB col, unsigned char alpha = 255);
void drawCircle(int x, int y, float r, colorRGB col);  
void drawCircleLines(int x, int y, float r, colorRGB col); 
void drawRing(Vector2 center, float iRad, float oRad, colorRGB col);
void drawRing(Vector2 center, float iRad, float oRad, colorRGB col, float alpha);
void drawGradientLineH(Vector2 a, Vector2 b, float thick, colorRGB col, float alphaA, float alphaB);

Vector2 measureTextEx(Font ft, string msg);

char* fileDialog(osdialog_file_action action, osdialog_filters* filters, const char* cdir = ".", const char* defName = nullptr);
