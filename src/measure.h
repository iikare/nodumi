#pragma once

#include <vector>
#include <set>
#include "note.h"
#include "timekey.h"
#include "log.h"

using std::vector;
using std::set;

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


    // basic structural properties
    set<int> tickMap;

    // persistent qualities to render
    vector<note*> notes;
    vector<timeSig> timeSignatures;
    vector<keySig> keySignatures;

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
