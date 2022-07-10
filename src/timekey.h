#pragma once

#include "log.h"
#include "enum.h"

class timeSig {
  public:
    timeSig() {
      top = 1;
      bottom = 1;
      qpm = 4;
      measure = -1;
      tick = -1;
    }

    timeSig(int t, int b, int tk) {
      top = t;
      bottom = b;
      qpm = 4.0 * static_cast<double>(t)/static_cast<double>(b);
      measure = -1;
      tick = tk;
    }

    timeSig(const timeSig& other) {
      top = other.top;
      bottom = other.bottom;
      qpm = other.qpm;
      measure = other.measure;
      tick = other.tick;
    }

    timeSig& operator= (const timeSig& other) {
      top = other.top;
      bottom = other.bottom;
      qpm = other.qpm;
      measure = other.measure;
      tick = other.tick;

      return *this;
    }
   
    bool operator== (const timeSig& other) const {
      return top == other.getTop() && bottom == other.getBottom();
    }
    
    
    void setMeasure(int m) { measure = m; }
    void setTick(int tk) { tick = tk; }
    int getTop() const { return top; }
    int getBottom() const { return bottom; }
    int getTick() const { return tick; }
    int getMeasure() const { return measure; }
    double getQPM() const { return qpm; }

  private:
    
    int top;
    int bottom;
    double qpm;
    int measure;
    int tick;
};

class keySig {
  public:
    keySig() {
      key = KEYSIG_NONE;
      accidentals = 0;
      measure = -1;
      tick = -1;
      prev = nullptr;

      findAccidentalsFromKey();
      findStartingIndex();
    }

    keySig(int k, bool m, int tk) {
      // key is in interval [-7,7]
      if (m) {
        logQ("ALERT");
        k = (k + 10) % 15 - 7; // upshift by three tones if minor
      }
      key = k;
      accidentals = 0;
      measure = -1;
      tick = tk;
      prev = nullptr;
      
      findAccidentalsFromKey();
      findStartingIndex();
    }

    keySig(const keySig& other) {
      key = other.key;
      measure = other.measure;
      tick = other.tick;
      prev = other.prev;
      
      findAccidentalsFromKey();
      findStartingIndex();
    }

    keySig& operator= (const keySig& other) {
      key = other.key;
      measure = other.measure;
      tick = other.tick;
      prev = other.prev;
      
      findAccidentalsFromKey();
      findStartingIndex();

      return *this;
    }

    bool operator== (const keySig& other) const {
      return key == other.key;
    }

    int getKey() {
      return key;
    }

    void setMeasure(int m) { measure = m; }
    void setTick(int tk) { tick = tk; }
    void setPrev(keySig* p) { prev = p; }

    keySig* getPrev() const { return prev; }

    int getSize() const;
    int getTick() const { return tick; }
    int getAcc() const { return accidentals; }
    int getIndex() const { return startingIndex; }

    bool isSharp() const { return accidentals > 0; }

    int measure;
  private:
    void findAccidentalsFromKey();
    void findStartingIndex();

    int key;
    int tick;
    int accidentals;
    int startingIndex;
    keySig* prev;
};
