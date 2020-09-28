#pragma once

#include <raylib.h>
#include "controller.h"

#define mWidth 1280
#define mHeight 720
#define mVersion "1.0a"
#define MIN_NOTE_IDX 21
#define MAX_NOTE_IDX 108
#define NOTE_RANGE 88
#define ITEM_HEIGHT 20
#define ITEM_WIDTH 100
#define MENU_MARGIN 10
#define MAIN_MENU_HEIGHT 19
#define COLOR_WIDTH 200
#define COLOR_HEIGHT 200

enum colorSelections {
  SELECT_BG,
  SELECT_LINE,
  SELECT_NONE
};

extern controller ctr;
extern Font font;
