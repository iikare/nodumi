#pragma once

#include <vector>
#include "note.h"
#include "timekey.h"
#include "log.h"

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

    // persistent qualities to render
    vector<note*> notes;
    vector<timeSig*> timeSignatures;
    vector<keySig*> keySignatures;

    // transient qualities not present in measure itself
    timeSig currentTime;
    keySig currentKey;

  private:

    double location;
    int number;
    int tick;
    int tickLength;

};
