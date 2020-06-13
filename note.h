#ifndef NOTE_H
#define NOTE_H

#include <string>
#include <algorithm>
#include "dpd/midifile/MidiFile.h"
#include "misc.h"

using namespace smf;
using std::string;

class note {
  public:
    note();
    
    void init(double itempo, double itime, double ipitch, double duration, unsigned char col);
    void shift(int shift_x);
    void updateTempo(int tempo);
    void scaleToWindow(int height, int range);

    int height = round(WIN_HEIGHT/NOTE_RANGE);
    int tempo;
    unsigned char col;
    int duration;
    int x;
    int y;
    bool render;
};

class mfile {
  public:
    mfile();
    ~mfile();

    note* getNotes();
    int getNoteRange();
    int getNoteCount();

    void shift(int shift_x);
    void updateTempo(int tempo_global);
    void scaleToWindow(int height);

    void load(string file);
  private:
    int noteCount;
    int noteMin;
    int noteMax;
    note* notes;
};

#endif
