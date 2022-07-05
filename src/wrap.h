#pragma once

#include <string>
#include <raylib.h>
#include "define.h"
#include "color.h"
#include "data.h"
#include "../dpd/osdialog/osdialog.h"

using std::string;

void clearBackground(const colorRGB& col);
void drawRectangle(float x, float y, float w, float h, const colorRGB& col);
void drawLine(int xi, int yi, int xf, int yf, const colorRGB& col);
void drawLineEx(int xi, int yi, int xf, int yf, float thick, const colorRGB& col);

void drawTextEx(const string& msg, const Vector2& pos, const colorRGB& col, 
                unsigned char alpha = 255, int size = 14, const string& font = DEFAULT_FONT);
void drawTextEx(const string& msg, int x, int y, const colorRGB& col, 
                unsigned char alpha = 255, int size = 14, const string& font = DEFAULT_FONT);

void drawSymbol(int sym, int size, int x, int y, const colorRGB& col, unsigned char alpha = 255, const string& font = MUSIC_FONT);

void drawCircle(int x, int y, float r, const colorRGB& col);  
void drawCircleLines(int x, int y, float r, const colorRGB& col); 
void drawRing(const Vector2& center, float iRad, float oRad, const colorRGB& col);
void drawRing(const Vector2& center, float iRad, float oRad, const colorRGB& col, float alpha);
void drawGradientLineH(const Vector2& a, const Vector2& b, float thick, const colorRGB& col, float alphaA, float alphaB);

const Vector2 measureTextEx(const string& msg, int size = 14, const string& font = DEFAULT_FONT);

char* fileDialog(osdialog_file_action action, osdialog_filters* filters, const char* cdir = ".", const char* defName = nullptr);
