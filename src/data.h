#pragma once

#define mWidth 1280
#define mHeight 740
#define mName "nodumi"
#define mVersion "0.01a"
#define MIN_NOTE_IDX 21
#define MAX_NOTE_IDX 108
#define MIDC_NOTE_IDX 60
#define NOTE_RANGE 88
#define ITEM_HEIGHT 20
#define ITEM_WIDTH 110
#define MENU_MARGIN 10
#define MAIN_MENU_HEIGHT 19
#define COLOR_WIDTH 200
#define COLOR_HEIGHT 200
#define SHEET_NOTEWIDTH 20
#define SHEET_RMARGIN 30
#define SHEET_LMARGIN 80
#define MIN_STAVE_IDX -23
#define MAX_STAVE_IDX 29

#define DEFAULT_FONT "YKLIGHT"
#define MUSIC_FONT   "LELAND"
#define TEXT_SPACING 0

#define SITE_LINK "https://iika.re/nodumi/"


#define TONIC_COUNT 12
#define KEY_COUNT 128

#define COLDIST_CIE00
#define KMEANS_ITERATIONS 2
#define MAX_UNIQUE_COLORS 10000


// already defined in <raylib.h>
//#define RAD2DEG M_PI/180.0f
//#define DEG2RAD 180.0f/M_PI

#define sinDeg(x) sin(x*RAD2DEG)
#define cosDeg(x) cos(x*RAD2DEG)
