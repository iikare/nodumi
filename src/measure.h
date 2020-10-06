#pragma once

#include <vector>
#include "note.h"

using std::vector;

class measureController {
  public:
    measureController() {
      location = -1;
      length = 0;
      notes = {};
    }
    measureController(double loc) {
      location = loc;
      length = 0;
      notes = {};
    }
  
    void findLength();
    
    double getLocation() { return location; }
    int getLength() { return length; }

    friend class midi;
  private:
    double location;
    int length;
    vector<note*> notes;

};
