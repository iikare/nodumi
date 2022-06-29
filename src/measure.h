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
      tick = -1;
      tickLength = 0;
      notes = {};
      timeSignatures = {};
      keySignatures = {};
    }
    measureController(double loc, int tk, int tkl) {
      location = loc;
      tick = tk;
      tickLength = tkl;
      notes = {};
      timeSignatures = {};
      keySignatures = {};
    }
  
    //void findLength();
    //void draw();
    
    double getLocation() { return location; }
    double getTick() { return tick; }

    void printNotes() {
      for (auto i : notes) {
        logQ(i->measure);
      }
    }

    friend class midi;
  private:

    double location;
    int tick;
    int tickLength;
    vector<note*> notes;
    vector<timeSig*> timeSignatures;
    vector<keySig*> keySignatures;

};
