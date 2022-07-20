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
    }

    note* getNextNote();
    note* getNextChordRoot();
    int getChordSize();
    
    bool isChordRoot();

    void findKeyPos(const keySig& key);

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

    int type = NOTE_NONE;
    int accType = ACC_NONE;
    int sheetY = MIN_NOTE_IDX;

    friend class trackController;
    vector<int> getLinePositions(note* now, note* next);

  private:
    note* prev;
    note* next;
    note* chordNext;


  
    struct staveVal {
      int offset;
      int acc;
    };

    // indexing from starting value of key (to keep on-key offset values constant)
    // 2D indices 1, 3, 6, 8, 10 are not in key
    static constexpr staveVal staveKeyMap[15][12] = {
                                                      { // KEYSIG_C 
                                                        {0, ACC_NONE }, {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, 
                                                        {2, ACC_NONE }, {3, ACC_NONE }, {3, ACC_SHARP}, {4, ACC_NONE }, 
                                                        {4, ACC_SHARP}, {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_CSHARP  
                                                        {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, {2, ACC_NONE }, 
                                                        {2, ACC_SHARP}, {3, ACC_SHARP}, {4, ACC_NONE }, {4, ACC_SHARP}, 
                                                        {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_NONE}, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_DFLAT 
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_FLAT }, {1, ACC_NONE }, 
                                                        {2, ACC_NONE }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_FLAT }, 
                                                        {4, ACC_NONE }, {5, ACC_FLAT }, {5, ACC_NONE }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_D  
                                                        {0, ACC_NONE }, {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, 
                                                        {2, ACC_SHARP}, {3, ACC_NONE }, {3, ACC_SHARP}, {4, ACC_NONE }, 
                                                        {4, ACC_SHARP}, {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_EFLAT
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_NONE }, {2, ACC_FLAT }, 
                                                        {2, ACC_NONE }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_FLAT }, 
                                                        {4, ACC_NONE }, {5, ACC_NONE }, {6, ACC_FLAT }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_E  
                                                        {0, ACC_NONE }, {1, ACC_NONE }, {1, ACC_SHARP}, {2, ACC_NONE }, 
                                                        {2, ACC_SHARP}, {3, ACC_NONE }, {3, ACC_SHARP}, {4, ACC_NONE }, 
                                                        {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_NONE }, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_F
                                                        {0, ACC_NONE }, {1, ACC_FLAT }, {1, ACC_NONE }, {2, ACC_FLAT }, 
                                                        {2, ACC_NONE }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_NONE }, 
                                                        {5, ACC_FLAT }, {5, ACC_NONE }, {6, ACC_FLAT }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_FSHARP  
                                                        {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, {2, ACC_NONE }, 
                                                        {2, ACC_SHARP}, {3, ACC_NONE }, {4, ACC_NONE }, {4, ACC_SHARP}, 
                                                        {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_NONE }, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_GFLAT
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_FLAT }, {1, ACC_NONE }, 
                                                        {2, ACC_FLAT }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_FLAT }, 
                                                        {4, ACC_NONE }, {5, ACC_FLAT }, {5, ACC_NONE }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_G
                                                        {0, ACC_NONE }, {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, 
                                                        {2, ACC_NONE }, {3, ACC_NONE }, {3, ACC_SHARP}, {4, ACC_NONE }, 
                                                        {4, ACC_SHARP}, {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_SHARP }, 
                                                      },
                                                      { // KEYSIG_AFLAT  
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_FLAT }, {1, ACC_NONE }, 
                                                        {2, ACC_NONE }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_FLAT }, 
                                                        {4, ACC_NONE }, {5, ACC_NONE }, {6, ACC_FLAT }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_A
                                                        {0, ACC_NONE }, {0, ACC_SHARP}, {1, ACC_NONE }, {1, ACC_SHARP}, 
                                                        {2, ACC_SHARP}, {3, ACC_NONE }, {3, ACC_SHARP}, {4, ACC_NONE }, 
                                                        {4, ACC_SHARP}, {5, ACC_SHARP}, {6, ACC_NONE }, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_BFLAT
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_NONE }, {2, ACC_FLAT }, 
                                                        {2, ACC_NONE }, {3, ACC_FLAT }, {3, ACC_NONE }, {4, ACC_NONE }, 
                                                        {5, ACC_FLAT }, {5, ACC_NONE }, {6, ACC_FLAT }, {6, ACC_NONE }, 
                                                      },
                                                      { // KEYSIG_B 
                                                        {0, ACC_NONE }, {1, ACC_NONE }, {1, ACC_SHARP}, {2, ACC_NONE }, 
                                                        {2, ACC_SHARP}, {3, ACC_NONE }, {4, ACC_NONE }, {4, ACC_SHARP}, 
                                                        {5, ACC_NONE }, {5, ACC_SHARP}, {6, ACC_NONE }, {6, ACC_SHARP}, 
                                                      },
                                                      { // KEYSIG_CFLAT 
                                                        {0, ACC_FLAT }, {0, ACC_NONE }, {1, ACC_FLAT }, {1, ACC_NONE }, 
                                                        {2, ACC_FLAT }, {3, ACC_FLAT }, {2, ACC_NONE }, {4, ACC_FLAT }, 
                                                        {4, ACC_NONE }, {5, ACC_FLAT }, {5, ACC_NONE }, {6, ACC_FLAT }, 
                                                      },
                                                    };
};

