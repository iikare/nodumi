#pragma once

#include <vector>
#include <set>
#include "sheetnote.h"
#include "enum.h"
#include "note.h"

using std::vector;
using std::set;
using std::pair;
using std::make_pair;

struct noteCmp {
    bool operator() (const sheetNote* a, const sheetNote* b) const {
        return a->oriNote->y < b->oriNote->y;
    }
};

struct chordCmp {
    bool operator() (const pair<int, vector<sheetNote*>>& a, const pair<int, vector<sheetNote*>>& b) const {
        return a.first < b.first;
    }
};

class sheetMeasure {
  public:

    void reset() {
      spacing.clear();
    }

    void addSpace(int space);

    void buildChordMap(vector<sheetNote>& vecNote);

  
    vector<pair<int, vector<sheetNote*>>> chords;

  private:
    vector<int> spacing;


};
