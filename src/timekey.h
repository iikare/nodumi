#pragma once

class timeSig {
  public:
    timeSig() {
      top = 0;
      bottom = 1;
      qpm = 4;
    }

    timeSig(int t, int b) {
      top = t;
      bottom = b;
      qpm = 4.0 * static_cast<double>(t)/b;
    }

    timeSig(const timeSig& other) {
      top = other.top;
      bottom = other.bottom;
    }

    bool operator== (const timeSig& other) {
      return top == other.top && bottom == other.bottom;
    }
    
    void operator= (const timeSig& other) {
      top = other.top;
      bottom = other.bottom;
    }
    
    int top;
    int bottom;
    double qpm;
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
    }

    keySig(int k, bool m) {
      key = k;
      isMinor = m;
    }

    keySig(const keySig& other) {
      isMinor = other.isMinor;
      key = other.key;
    }

    bool operator== (const keySig& other) {
      return key == other.key && isMinor == other.isMinor;
    }

    int getKey() {
      if (!isMinor) {
        return key;
      }
      else {
        return toMajorKey(key);
      }
    }
    
    bool isMinor;
  private:
    int key;

    int toMajorKey(int minorKey) {
      // stub
      return minorKey;
    }
};
