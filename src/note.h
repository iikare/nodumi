#pragma once

#include <vector>
#include "timekey.h"

using std::vector;

class note {
  public:
    note() {
      number = -1;
      size = 0x00000000;
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

    }

    note* getNextNote();
    note* getNextChordRoot();
    int getChordSize();
    
    void setKeySig(keySig* ks) { key = ks; }
    keySig* getKeySig() { return key; };

    bool isChordRoot();

    void findSize(vector<int>& noteChart);
    void findSheetY();
    
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

    friend class trackController;
    friend vector<int> getLinePositions(note* now, note* next);

  private:
  
    note* prev;
    note* next;
    note* chordNext;

    keySig* key;

};

enum noteSize {
  NOTE_LARGE,
  NOTE_WHOLE,
  NOTE_HALF,
  NOTE_QUARTER,
  NOTE_8,
  NOTE_16,
  NOTE_32,
  NOTE_64,
  NOTE_128,
  NOTE_NONE
};
