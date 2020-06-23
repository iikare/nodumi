#ifndef NOTE_H
#define NOTE_H

#include <string>
#include <algorithm>
#include "../dpd/midifile/MidiFile.h"
#include "misc.h"

using namespace smf;
using std::string;

class note {
  public:
    note();
    ~note();
    note(const note& nNote);
    
    void operator = (const note& nNote);
    
    void init(int track, double tempo, double x, int y, double duration);
    void shiftX(double shiftX);
    void shiftTime(double timeInc);
    void scaleTime(double timeScale);
    
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
    int getTrackCount();
    double getLastTick();
    double getTimeScale();

    void shiftX(double shiftX);
    void shiftTime(double timeInc);
    void scaleTime(double timeScale);

    void load(string file);
  private:
    int trackCount;
    int noteCount;
    int noteMin;
    int noteMax;
    double timeScale;
    double lastTick;
    note* notes;
};

#endif
