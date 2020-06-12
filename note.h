#ifndef NOTE_H
#define NOTE_H

#include <string>
#include "dpd/midifile/MidiFile.h"

using namespace smf;
using std::string;

class note {
  public:
    note();
    
    void init(double itempo, double itime, double ipitch, double duration, unsigned char col);
    void shift(int shift_x);
    void update(int tempo);

    const static int height = 14; // TODO: make scale on window size
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
    int getNoteCount();

    void shift(int shift_x);
    void update(int tempo_global);
    void load(string file);
  private:
    int noteCount;
    note* notes;
};

#endif
