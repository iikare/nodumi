#include <vector>
#include "note.h"
#include "timekey.h"

using std::vector;

enum UMOTypes {
  UMO_NOTE,
  UMO_KEY,
  UMO_TIME,
  UMO_TEMPO,
  UMO_NONE
};

class UMO {
  public:
    UMO(note* nptr, int size) {
      type = UMO_NOTE;
      tick = nptr->tick;
      unitWidth = size;
      n = {};
      n.push_back(nptr);
      ts = nullptr;
      ks = nullptr;
    }
    UMO(timeSig* tsptr, int size) {
      type = UMO_TIME;
      tick = tsptr->tick;
      unitWidth = size;
      n = {};
      ts = tsptr;
      ks = nullptr;
    }
    UMO(keySig* ksptr, int size) {
      type = UMO_KEY;
      tick = ksptr->tick;
      unitWidth = size;
      n = {};
      ts = nullptr;
      ks = ksptr;
    }

    int getSize() const { return unitWidth; }
    int getTick() const { return tick; }
    int getType() const { return type; }
    int getNumEvents() const;

    void* getRawEvent();

    void addNote(note* nNote);
  private:
    int type;
    int tick;
    int unitWidth;
    vector<note*> n;
    timeSig* ts;
    keySig* ks;
};
