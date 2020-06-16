
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

bool hoverOnNote(int mouseX, int mouseY, int noteX, int noteY, int noteWidth, int noteHeight);

#endif
