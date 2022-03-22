#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "../dpd/midifile/MidiFile.h"
#include "note.h"
#include "track.h"
#include "timekey.h"
#include "sheetctr.h"
#include "measure.h"
#include "color.h"
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
      measureMap = {};
      measureTickMap = {};
      tickMap = {};
      sheetData.reset();

      tracks.resize(1);
      
      trackCount = 0;
      noteCount = 0;
      lastTime = 0;
      lastTick = 0;

      tpq = 0;
    }

    void load(string file);
    
    vector<int>* getLineVerts() { return &lineVerts; }
    int findMeasure(int offset);
    int findParentMeasure(int measure);

    vector<note> notes;
    sheetController sheetData;
    vector<measureController> measureMap;
    vector<measureController> measureTickMap;

    friend class midiInput;
    friend class controller;

    vector<pair<int, double>> trackHeightMap;
  private:
    vector<pair<double, int>> tempoMap;
    vector<trackController> tracks;
    vector<int> lineVerts;
    vector<int> tickMap;

    int getTrackCount() { return trackCount; }
    int getNoteCount() { return noteCount; }
    int getLastTime() { return lastTime; }
    int getTempo(int offset);
    
    void buildLineMap();
    void buildTickMap();

    void findMeasure(note& idxNote);
    void findKeySig(note& idxNote);

    MidiFile midifile;

    int trackCount;
    int noteCount;

    double lastTime;
    int lastTick;

    int tpq;
 

};
