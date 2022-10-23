#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
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
using std::function;

// using std::function was disastrous
struct tickCmp {
    bool operator() (const pair<int, int>& a, const pair<int, int>& b) const {
        return a.first < b.first;
    }
};

struct itemStartCmp{
    bool operator() (const pair<int, int>& a, const pair<int, int>& b) const {
        return a.first <= b.first;
    }
};

class midi {
  public:
    midi() {
      notes = {};
      message = {};
      tempoMap = {};
      tracks = {};
      trackHeightMap = {};
      lineVerts = {};
      measureMap = {};
      timeSignatureMap = {};
      keySignatureMap = {};
      tickSet = {};
      itemStartSet = {};


      tracks.resize(1);
      
      trackCount = 0;
      noteCount = 0;
      lastTime = 0;
      lastTick = 0;

      tpq = 0;

    }

    void clear();
    void load(string file, stringstream& buf);
    void load(stringstream& buf);
    
    vector<int>* getLineVerts() { return &lineVerts; }
    int findMeasure(int offset);
    
    int getMinTickLen() const { return tickNoteTransform[tickNoteTransformLen-1]*tpq; }
    int getTrackCount() const { return trackCount; }
    int getNoteCount() const { return noteCount; }
    int getLastTick() const { return lastTick; }
    int getLastTime() const { return lastTime; }
    int getTPQ() const { return tpq; }
    int getTempo(int offset);
    vector<trackController>& getTracks() { return tracks;}
    
    void setNoteCount(int nc) { noteCount = nc; }

    vector<note> notes;
    vector<pair<double, vector<unsigned char>>> message;

    sheetController sheetData;
    vector<measureController> measureMap;

    vector<pair<int, double>> trackHeightMap;

    friend class midiInput;
    
  private:
    vector<pair<double, int>> tempoMap;
    vector<trackController> tracks;
    vector<int> lineVerts;
    vector<int> tickMap;
    vector<pair<int, timeSig>> timeSignatureMap;
    vector<pair<int, keySig>> keySignatureMap;
    
    set<pair<int, int>, tickCmp> tickSet;
    set<pair<int, int>, itemStartCmp> itemStartSet;

    void addTimeSignature(int position, const timeSig& timeSignature);
    timeSig getTimeSignature(int offset);
    
    void addKeySignature(int position, const keySig& keySignature);
    void linkKeySignatures();
    keySig getKeySignature(int offset);
    keySig eventToKeySignature(int keySigType, bool isMinor, int tick);
    
    void buildLineMap();
    void buildTickSet();
    void buildMessageMap(const MidiFile& mf);

    int trackCount;
    int noteCount;

    double lastTime;
    int lastTick;
    int tpq;
 
    static constexpr double tickNoteTransform[13] =
                                                    {
                                                      8, 6, 4, 3, 2, 1.5, 1,
                                                      0.75, 0.5, 0.375, 0.25, 0.125, 0.0625,
                                                    };
    static constexpr int tickNoteTransformLen = 13;
};
