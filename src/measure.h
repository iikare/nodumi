#pragma once

#include <vector>
#include "note.h"
#include "timekey.h"

using std::vector;

class measureController {
  public:
    measureController() {
      location = -1;
      length = 0;
      tick = -1;
      tickLength = 0;
      parentMeasure = 0;
      displayX = 0;
      displayLength = 0;
      notes = {};
      timeSignatures = {};
      keySignatures = {};
    }
    measureController(double loc, int tk, int tkl) {
      location = loc;
      length = 0;
      tick = tk;
      tickLength = tkl;
      parentMeasure = 0;
      displayX = 0;
      displayLength = 0;
      notes = {};
      timeSignatures = {};
      keySignatures = {};
    }
  
    void findLength();
    void draw();
    
    double getLocation() { return location; }
    int getLength() { return length; }

    int getDisplayLocation() { return displayX; }
    int getDisplayLength() { return displayLength; }
    int getParent() { return parentMeasure; }

    friend class midi;
  private:
    double location;
    int length;
    int tick;
    int tickLength;
    int parentMeasure;
    int displayX;
    int displayLength;
    vector<note*> notes;
    vector<timeSig*> timeSignatures;
    vector<keySig*> keySignatures;

};
