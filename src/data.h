#pragma once

#include "data_str.h"

#define W_WIDTH 1280
#define W_HEIGHT 740
#define W_NAME "nodumi"
#define W_VER "0.01a"

#define MKI_VER_MAJOR '1'
#define MKI_VER_MINOR 'b'

#define UNK_CST 500

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
#define SHEET_RMARGIN 30
#define SHEET_LMARGIN 80
#define MIN_STAVE_IDX -23
#define MAX_STAVE_IDX 29
#define MAX_HAND_RANGE 16

#define DEFAULT_FONT "YKLIGHT"
#define GLYPH_FONT "LELAND"
#define TEXT_SPACING 0

#define FILTER_FILE "midi/mki:mid,midi,mki"
#define FILTER_IMAGE "image:png,jpeg,jpg"
#define FILTER_SAVE "mki:mki"

#define SITE_LINK "https://iika.re/nodumi/"

#define TONIC_COUNT 12
#define KEY_COUNT 128

#define KMEANS_ITERATIONS 2
#define MAX_UNIQUE_COLORS 10000

#define FFT_MIN_FREQ 20
#define FFT_MAX_FREQ 44100
#define FFT_BIN_WIDTH 10
#define FFT_AC_BINS 8

#define VORONOI_MAX_POINTS 496

// already defined in <raylib.h>
// #define RAD2DEG M_PI/180.0f
// #define DEG2RAD 180.0f/M_PI

#define sinDeg(x) sin(x* RAD2DEG)
#define cosDeg(x) cos(x* RAD2DEG)
