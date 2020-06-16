
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
#define MAINMENU_HEIGHT 20
#define ITEM_HEIGHT 20
#define ITEM_WIDTH 100
#define TEXT_OFFSET 4

bool hoverOnNote(int mouseX, int mouseY, int noteX, int noteY, int noteWidth, int noteHeight);
void getMenuLocation(int mainW, int mainH, int cnX, int cnY,
                     int& rcX, int& rcY, const int rcW, const int rcH);


#endif
