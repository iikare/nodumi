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
    }
    measureController(int num, double loc, int tk, int tkl, timeSig cTime) {
      location = loc;
      number = num;
      tick = tk;
      tickLength = tkl;
      notes = {};
      timeSignatures = {};
      keySignatures = {};
      currentTime = cTime;
    }
  
    //void findLength();
    //void draw();
    
    double getLocation() const { return location; }
    int getNumber() const { return number; }
    int getTick() const { return tick; }

    vector<note*> notes;
    vector<timeSig*> timeSignatures;
    vector<keySig*> keySignatures;

    timeSig currentTime;

  private:

    double location;
    int number;
    int tick;
    int tickLength;

};
