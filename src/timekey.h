#pragma once

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

    bool operator== (const timeSig& other) {
      return top == other.top && bottom == other.bottom;
    }
    
    void operator= (const timeSig& other) {
      top = other.top;
      bottom = other.bottom;
      qpm = other.qpm;
    }
    
    void setMeasure(int m) { measure = m; }
    void setTick(int tk) { tick = tk; }
    int getSize();
    
    int top;
    int bottom;
    double qpm;
    int measure;
    int tick;
};

enum keySignature {
  KEYSIG_CFLAT,
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
  KEYSIG_NONE
};

class keySig {
  public:
    keySig() {
      isMinor = false;
      key = KEYSIG_C;
      accidentals = 0;
      measure = -1;
      tick = -1;
      prev = nullptr;

      findAccidentalsFromKey();
    }

    keySig(int k, bool m, int tk) {
      key = k;
      accidentals = 0;
      isMinor = m;
      measure = -1;
      tick = tk;
      prev = nullptr;
      
      findAccidentalsFromKey();
    }

    keySig(const keySig& other) {
      isMinor = other.isMinor;
      key = other.key;
      measure = other.measure;
      tick = other.tick;
      prev = other.prev;
      findAccidentalsFromKey();
    }

    bool operator== (const keySig& other) {
      return key == other.key && isMinor == other.isMinor;
    }

    int getKey() {
      return key;
      
      // junk
      if (!isMinor) {
        return key;
      }
      else {
        return toMajorKey(key);
      }
    }

    void setMeasure(int m) { measure = m; }
    void setTick(int tk) { tick = tk; }
    void setPrev(keySig* p) { prev = p; }

    keySig* getPrev() { return prev; }

    void findAccidentalsFromKey();
    int getSize();

    bool isMinor;
    int measure;
    int tick;
  private:
    int key;
    int accidentals;
    keySig* prev;

    int toMajorKey(int minorKey) {
      // stub
      return minorKey;
    }
};
