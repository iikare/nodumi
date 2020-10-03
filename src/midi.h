#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "../dpd/midifile/MidiFile.h"
#include "note.h"
#include "track.h"
#include "log.h"

using namespace smf;
using std::string;
using std::to_string;
using std::vector;
using std::pair;
using std::make_pair;

class midi {
  public:
    midi() {
      notes = {};
      tempoMap = {};
      tracks = {};
      trackHeightMap = {};
      lineVerts = {};
      tracks.resize(1);
      trackCount = 0;
      noteCount = 0;
      lastTick = 0;
    }

    void load(string file);
    
    vector<int>* getLineVerts() { return &lineVerts; }

    vector<note> notes;

    friend class midiInput;
    friend class controller;
  private:
    vector<pair<double, int>> tempoMap;
    vector<trackController> tracks;
    vector<pair<int, double>> trackHeightMap;
    vector<int> lineVerts;

    int getTrackCount() { return trackCount; }
    int getNoteCount() { return noteCount; }
    int getLastTick() { return lastTick; }
    int getTempo(int idx);
    
    void buildLineMap();


    int trackCount;
    int noteCount;

    double lastTick;
 

};
