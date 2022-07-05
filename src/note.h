#pragma once

#include <vector>
#include <set>
#include "timekey.h"
#include "data.h"
#include "enum.h"

using std::vector;
using std::set;

struct tickCmp; // for note comparison with the tick-to-length set (comparator struct)

class note {
  public:
    note() {
      number = -1;
      size = 0;
      tick = 0;
      tickDuration = 0;
      track = 0;
      measure = 0;

      duration = 0;
      x = 0;
      
      y = 0;
      velocity = 0;
      
      isOn = false;
      isLastOnTrack = false;
      
      prev = nullptr;
      next = nullptr;
      chordNext = nullptr;
      key = nullptr;

      type = NOTE_NONE;
    }

    note* getNextNote();
    note* getNextChordRoot();
    int getChordSize();
    
    void setKeySig(keySig* ks) { key = ks; }
    keySig* getKeySig() { return key; };

    bool isChordRoot();

    void findSheetParameters();

    void findSize(const set<pair<int,int>, tickCmp>& tickSet);
    
    int number;
    int size; 
    int tick;
    int tickDuration;
    int track;
    int measure;
    double duration;
    double x;
    int y;
    int velocity;
    bool isOn;
    bool isLastOnTrack;

    int type;

    friend class trackController;
    friend vector<int> getLinePositions(note* now, note* next);

  private:
  
    note* prev;
    note* next;
    note* chordNext;

    keySig* key;

};

