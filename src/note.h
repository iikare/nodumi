#ifndef NOTE_H
#define NOTE_H

#include <string>
#include <vector>
#include <algorithm>
#include "../dpd/midifile/MidiFile.h"
#include "misc.h"

using namespace smf;
using std::string;
using std::vector;
using std::pair;

class note {
  public:
    note();
    ~note();
    note(const note& nNote);
    
    void operator = (const note& nNote);
    
    void shiftX(double shiftX);
    void shiftTime(double timeInc);
    void scaleTime(double timeScale);

    
    int track;
    double duration;
    double x;
    int y;
    int velocity;
    double time;
    bool isOn;

};

class mfile {
  public:
    mfile();
    mfile(int bufSize);
    ~mfile();

    note* getNotes();
    int getNoteRange();
    int getNoteCount();
    int getTrackCount();
    double getLastTick();
    double getLastTime();
    double getTimeScale();
    vector<pair<double, int>> getTempoMap() { return tempoMap; }

    note findCurrentNote();
    int findCurrentTempo();

    void shiftX(double shiftX);
    void shiftTime(double timeInc);
    void scaleTime(double timeScale);

    friend class MidiInput;
    friend void loadFileMKI(string path, mfile*& input, vector<colorRGB>& colorVecA, vector<colorRGB>& colorVecB, colorRGB& bg, int& displayMode, int& colorMode, bool& drawLine, bool& songTime, bool& invertColor);

    void load(string file);
  private:
    int trackCount;
    int noteCount;
    int noteMin;
    int noteMax;
    double timeScale;
    double lastTick;
    double lastTime;

    vector<pair<double, int>> tempoMap;
    note* notes;
};

#endif
