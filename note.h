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
    void shiftX(int shiftX);
    void shiftTime(int timeInc);

    int height = round(WIN_HEIGHT/NOTE_RANGE);
    int tempo;
    unsigned char col;
    int duration;
    int x;
    int y;
};

class mfile {
  public:
    mfile();
    ~mfile();

    note* getNotes();
    int getNoteRange();
    int getNoteCount();

    void shiftX(int shiftX);
    void shiftTime(int timeInc);

    void load(string file);
  private:
    int noteCount;
    int noteMin;
    int noteMax;
    note* notes;
};

#endif
