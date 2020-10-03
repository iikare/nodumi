#pragma once

#include <vector>

using std::vector;

class note {
  public:
    note() {
      number = -1;
      tick = 0;
      tickDuration = 0;
      track = 0;
      duration = 0;
      x = 0;
      y = 0;
      velocity = 0;
      isOn = false;
      prev = nullptr;
      next = nullptr;
      chordNext = nullptr;

    }

    note* getNextNote();
    note* getNextChordRoot();
    int getChordSize();

    bool isChordRoot();
    
    int number;
    int tick;
    int tickDuration;
    int track;
    double duration;
    double x;
    int y;
    int velocity;
    bool isOn;

    friend class trackController;
    friend vector<int> getLinePositions(note* now, note* next);

  private:
  
    note* prev;
    note* next;
    note* chordNext;

};
