#pragma once

class note {
  public:
    note() {
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
    
    bool isChordRoot();
    
    int track;
    double duration;
    double x;
    int y;
    int velocity;
    bool isOn;

    friend class trackController;

    int getChordSize();
  private:
  
    note* prev;
    note* next;
    note* chordNext;

};
