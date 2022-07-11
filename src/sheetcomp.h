#pragma once

#include <vector>
#include <set>
#include "enum.h"
#include "note.h"

using std::vector;
using std::set;
using std::pair;
using std::make_pair;

class sheetMeasure {
  public:

    void reset() {
      spacing.clear();
    }

    void addSpace(int space);

  private:
    vector<int> spacing;

};

struct noteCmp {
    bool operator() (const note* a, const note* b) const {
        return a->y < b->y;
    }
};

struct chordCmp {
    bool operator() (const pair<int, vector<note*>>& a, const pair<int, vector<note*>>& b) const {
        return a.first < b.first;
    }
};

class sheetChord {
  public:


    void addNote(note& n);

  
    set<pair<int, vector<note*>>, chordCmp> chord;

  private:


};
