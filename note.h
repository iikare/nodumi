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
    
    void init(int track, double tempo, double x, int y, double duration);
    void shiftX(int shiftX);
    void shiftTime(int timeInc, double timeScale);
    void scaleTime(double timeScale);

    int height = round(WIN_HEIGHT/NOTE_RANGE);
    int track;
    double tempo;
    double duration;
    double x;
    int y;
};

class mfile {
  public:
    mfile();
    ~mfile();

    note* getNotes();
    int getNoteRange();
    int getNoteCount();
    int getTimeScale();

    void shiftX(int shiftX);
    void shiftTime(int timeInc);
    void scaleTime(double timeScale);

    void load(string file);
  private:
    int noteCount;
    int noteMin;
    int noteMax;
    double timeScale;
    note* notes;
};

#endif
