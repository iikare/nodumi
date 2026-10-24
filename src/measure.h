#pragma once

#include <deque>
#include <vector>

#include "log.h"
#include "note.h"
#include "sheetnote.h"
#include "timekey.h"

using std::deque;
using std::vector;

class measureController {
 public:
  measureController() {
    location = -1;
    number = -1;
    tick = -1;
    tickLength = 0;
    notes = {};
    timeSignatures = {};
    keySignatures = {};
    currentTime = timeSig();
    currentKey = keySig();
  }
  measureController(int num, double loc, int tk, int tkl, const timeSig& cTime, const keySig& cKey) {
    location = loc;
    number = num;
    tick = tk;
    tickLength = tkl;
    notes = {};
    timeSignatures = {};
    keySignatures = {};
    currentTime = cTime;
    currentKey = cKey;
  }

  void clear();

  double getLocation() const { return location; }
  int getNumber() const { return number; }
  int getTick() const { return tick; }
  int getTickLen() const { return tickLength; }

  void addNote(note& note);

  // persistent qualities to render
  vector<int> notes;
  vector<timeSig> timeSignatures;
  vector<keySig> keySignatures;

  // notes sustained across the start or end of this measure
  vector<int> tie_in_notes;
  vector<int> tie_out_notes;

  // adjustments for sheet rendering, rebuilt from the note lists by the sheet controller
  deque<sheetNote> displayNotes;

  // transient qualities not present in measure itself
  timeSig currentTime;
  keySig currentKey;

 private:
  double location;
  int number;
  int tick;
  int tickLength;
};
