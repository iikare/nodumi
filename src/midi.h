#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "../dpd/midifile/MidiFile.h"
#include "color.h"
#include "line.h"
#include "log.h"
#include "measure.h"
#include "note.h"
#include "sheetctr.h"
#include "timekey.h"
#include "track.h"

using namespace smf;

using std::deque;
using std::ifstream;
using std::multiset;
using std::pair;
using std::string;
using std::stringstream;
using std::vector;

// using std::function was disastrous
struct tickCmp {
  bool operator()(const pair<int, int>& a, const pair<int, int>& b) const { return a.first < b.first; }
};

struct itemStartCmp {
  bool operator()(const pair<int, int>& a, const pair<int, int>& b) const { return a.first <= b.first; }
};

class midi {
 public:
  midi() {
    notes = {};
    message = {};
    lines = {};
    tempoMap = {};
    tracks = {};
    trackHeightMap = {};
    measureMap = {};
    timeSignatureMap = {};
    keySignatureMap = {};
    tickSet = {};
    itemStartSet = {};

    tracks.resize(2);

    velocityBounds = make_pair(127, 0);

    trackCount = 0;
    noteCount = 0;
    lastTime = 0;
    lastTick = 0;

    tpq = 48;
  }

  void clear();
  void load(string file, stringstream& buf);
  void load(stringstream& buf);

  const vector<lineData>& getLines() { return lines; }
  int findMeasure(int offset) const;

  int getMinTickLen() const { return tickNoteTransform[tickNoteTransformLen - 1] * tpq; }
  int getTrackCount() const { return trackCount; }
  int getNoteCount() const { return noteCount; }
  int getLastTick() const { return lastTick; }
  int getLastTime() const { return lastTime; }
  int getTPQ() const { return tpq; }
  int getTempo(int offset) const;
  vector<trackController>& getTracks() { return tracks; }
  const set<pair<int, int>, tickCmp>& getTickSet() const { return tickSet; }

  void setNoteCount(int nc) { noteCount = nc; }
  void setTPQ(int new_tpq) { tpq = new_tpq; }

  vector<note> notes;
  multiset<pair<double, vector<unsigned char>>> message;

  vector<lineData> lines;
  sheetController sheetData;
  deque<measureController> measureMap;

  vector<pair<int, double>> trackHeightMap;

  // min, max
  pair<int, int> velocityBounds;

  friend class midiInput;
  friend class trackController;

 private:
  vector<pair<double, int>> tempoMap;
  vector<trackController> tracks;
  vector<int> tickMap;
  vector<pair<double, timeSig>> timeSignatureMap;
  vector<pair<double, keySig>> keySignatureMap;

  set<pair<int, int>, tickCmp> tickSet;
  set<pair<int, int>, itemStartCmp> itemStartSet;

  void addTimeSignature(double position, const timeSig& timeSignature);
  timeSig getTimeSignature(double offset);

  void addKeySignature(double position, const keySig& keySignature);
  void linkKeySignatures();
  keySig getKeySignature(double offset);
  keySig eventToKeySignature(int keySigType, bool isMinor, int tick);
  int findKeySig();

  void buildLineMap();
  void buildTickSet();
  void buildMessageMap(const MidiFile& mf);

  int trackCount;
  int noteCount;

  double lastTime;
  int lastTick;
  int tpq;

  static constexpr double tickNoteTransform[13] = {
      8, 6, 4, 3, 2, 1.5, 1, 0.75, 0.5, 0.375, 0.25, 0.125, 0.0625,
  };
  static constexpr int tickNoteTransformLen = 13;
};
