#pragma once

#include <set>
#include <vector>

#include "log.h"
#include "note.h"
#include "sheetnote.h"
#include "timekey.h"

using std::set;
using std::vector;

class measureController {
 public:
  measureController() {
    location = -1;
    number = -1;
    tick = -1;
    tickLength = 0;
    tickMap = {};
    notes = {};
    timeSignatures = {};
    keySignatures = {};
    currentTime = timeSig();
    currentKey = keySig();

    buildTickMap();
  }
  measureController(int num, double loc, int tk, int tkl, const timeSig& cTime, const keySig& cKey) {
    location = loc;
    number = num;
    tick = tk;
    tickLength = tkl;
    tickMap = {};
    notes = {};
    timeSignatures = {};
    keySignatures = {};
    currentTime = cTime;
    currentKey = cKey;

    buildTickMap();
  }

  void clear();

  double getLocation() const { return location; }
  int getNumber() const { return number; }
  int getTick() const { return tick; }
  int getTickLen() const { return tickLength; }

  void addNote(note& note);

  // basic structural properties
  set<int> tickMap;

  // persistent qualities to render
  vector<note*> notes;
  vector<timeSig> timeSignatures;
  vector<keySig> keySignatures;

  // adjustments for sheet rendering
  vector<sheetNote> displayNotes;

  // transient qualities not present in measure itself
  timeSig currentTime;
  keySig currentKey;

 private:
  void buildTickMap();

  double location;
  int number;
  int tick;
  int tickLength;
};
