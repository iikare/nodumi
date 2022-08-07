#pragma once

#include <vector>
#include <set>
#include "sheetnote.h"
#include "measure.h"
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

struct flagData {
  int startY;
  int endY;
  int flagType;
  int flagDir;
};

struct chordData {
  int leftWidth;
  int rightWidth;

  vector<flagData> flags;

  int getStemPosition() const { return leftWidth; }
  int getSize() const { return leftWidth + rightWidth; }
};

class sheetMeasure {
  public:

    void reset() {
      spacing.clear();
    }

    void addSpace(int space);
    bool hasStem(int chordNum) const;
    int hasFlag(int chordNum) const;
    int getFlagType(const int noteType) const;

    void buildChordMap(vector<sheetNote>& vecNote);
    void buildFlagMap();

    void setParent(measureController& m) { measure = &m; }

    int getSpacingCount() const;
 
    vector<chordData> chordData;
    vector<pair<int, vector<sheetNote*>>> chords;
    measureController* measure;

  private:
    vector<int> spacing;



};
