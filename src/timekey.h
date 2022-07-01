#pragma once

#include "log.h"

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

      return *this;
    }
   
    bool operator== (const timeSig& other) const {
      return top == other.top && bottom == other.bottom;
    }
    
    
    void setMeasure(int m) { measure = m; }
    void setTick(int tk) { tick = tk; }
    int getTick() { return tick; }
    int getMeasure() { return measure; }
    int getQPM() { return qpm; }
    int getSize();

  private:
    
    int top;
    int bottom;
    double qpm;
    int measure;
    int tick;
};

enum keySignature {
  KEYSIG_C,
  KEYSIG_CSHARP,
  KEYSIG_DFLAT,
  KEYSIG_D,
  KEYSIG_EFLAT,
  KEYSIG_E,
  KEYSIG_F,
  KEYSIG_FSHARP,
  KEYSIG_GFLAT,
  KEYSIG_G,
  KEYSIG_AFLAT,
  KEYSIG_A,
  KEYSIG_BFLAT,
  KEYSIG_B,
  KEYSIG_CFLAT,
  KEYSIG_NONE
};

class keySig {
  public:
    keySig() {
      key = KEYSIG_NONE;
      accidentals = 0;
      startingIndex = -1;
      measure = -1;
      tick = -1;
      prev = nullptr;

      findAccidentalsFromKey();
      findStartingIndex();
    }

    keySig(int k, bool m, int tk) {
      // key is in interval [-7,7]
      if (m) {
        k = (k + 10) % 15 - 7; // upshift by three tones if minor
      }
      key = k;
      accidentals = 0;
      startingIndex = -1;
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

    keySig* getPrev() { return prev; }

    int getSize();
    int getTick() { return tick; }
    int getAcc() { return accidentals; }
    int getStartingIndex() { return startingIndex; }

    bool isSharp() { return accidentals > 0; }

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
