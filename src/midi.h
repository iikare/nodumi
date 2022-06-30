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
using std::max;
using std::ifstream;
using std::stringstream;

class midi {
  public:
    midi() {
      notes = {};
      tempoMap = {};
      tracks = {};
      trackHeightMap = {};
      lineVerts = {};
      measureMap = {};
      timeSignatureMap = {};
      keySignatureMap = {};
      tickMap = {};

      tracks.resize(1);
      
      trackCount = 0;
      noteCount = 0;
      lastTime = 0;
      lastTick = 0;

      tpq = 0;
    }

    void load(string file, stringstream& buf);
    
    vector<int>* getLineVerts() { return &lineVerts; }
    int findMeasure(int offset);
    
    int getLastTime() { return lastTime; }
    vector<trackController>& getTracks() { return tracks;}
    
    void setNoteCount(int nc) { noteCount = nc; }

    vector<note> notes;

    sheetController sheetData;
    vector<measureController> measureMap;

    //friend class midiInput;
    friend class controller;

    vector<pair<int, double>> trackHeightMap;
  private:
    vector<pair<double, int>> tempoMap;
    vector<trackController> tracks;
    vector<int> lineVerts;
    vector<int> tickMap;
    vector<pair<int, timeSig>> timeSignatureMap;
    vector<pair<int, keySig>> keySignatureMap;
    
    int getTrackCount() { return trackCount; }
    int getNoteCount() { return noteCount; }
    int getLastTick() { return lastTick; }
    int getTempo(int offset);

    void addTimeSignature(int position, int tick, timeSig timeSignature);
    timeSig getTimeSignature(int offset);
    
    void addKeySignature(int position, int tick, keySig keySignature);
    void linkKeySignatures();
    keySig getKeySignature(int offset);
    keySig eventToKeySignature(int keySigType, bool isMinor);
    
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
