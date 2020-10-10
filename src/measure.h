#pragma once

#include <vector>
#include "note.h"

using std::vector;

class measureController {
  public:
    measureController() {
      location = -1;
      length = 0;
      parentMeasure = 0;
      displayX = 0;
      displayLength = 0;
      notes = {};
    }
    measureController(double loc) {
      location = loc;
      length = 0;
      parentMeasure = 0;
      displayX = 0;
      displayLength = 0;
      notes = {};
    }
  
    void findLength();
    
    double getLocation() { return location; }
    int getLength() { return length; }

    int getDisplayLocation() { return displayX; }
    int getDisplayLength() { return displayLength; }
    int getParent() { return parentMeasure; }

    friend class midi;
  private:
    double location;
    int length;
    int parentMeasure;
    int displayX;
    int displayLength;
    vector<note*> notes;

};
