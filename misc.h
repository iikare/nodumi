
#ifndef MISC_H
#define MISC_H

#define VERSION "0.01a"
#define MIN_NOTE_IDX 21
#define MAX_NOTE_IDX 108
#define NOTE_RANGE 88
#define WIN_HEIGHT 720
#define WIN_WIDTH 1280
#define TIME_MODIFIER 8
#define CTRL_MODIFIER 10
#define MENU_MARGIN 10
#define MAIN_MENU_HEIGHT 19
#define ITEM_HEIGHT 19
#define ITEM_WIDTH 100
#define TEXT_OFFSET 4
#define FILE_MENU_WIDTH 22
#define EDIT_X FILE_MENU_WIDTH
#define EDIT_MENU_WIDTH 25
#define VIEW_X EDIT_X + EDIT_MENU_WIDTH
#define VIEW_MENU_WIDTH 28
#define COLOR_WIDTH 100
#define COLOR_HEIGHT 100

#include <iostream>
#include "color.h"
#include "box.h"

using std::string;

bool hoverOnBox(int mouseX, int mouseY, int noteX, int noteY, int noteWidth, int noteHeight);
bool hoverOnBox(point mouse, int noteX, int noteY, int noteWidth, int noteHeight);
bool hoverOnBox(int mouseX, int mouseY, rect box);
bool hoverOnBox(point mouse, rect box);
bool hoverOnBox(int mouseX, int mouseY, int noteX, int noteY, int size);
void getMenuLocation(point XY, point mouseXY, int& rcX, int& rcY, const int rcW, const int rcH);
void getMenuLocation(int mainW, int mainH, int cnX, int cnY, int& rcX, int& rcY, const int rcW, const int rcH);
string getSongPercent(int pos, double total, bool end);
void getColorSelectLocation(int mainW, int mainH, int& cpX, int& cpY, int rcX, int rcY, int xBuf, int yBuf);
double getDistance(int x1, int y1, int x2, int y2);
colorRGB getHueByAngle(int x1, int y1, int x2, int y2);



#endif
