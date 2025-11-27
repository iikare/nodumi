#pragma once

#include <deque>
#include <set>
#include <vector>

#include "enum.h"
#include "measure.h"
#include "note.h"
#include "sheetnote.h"

using std::deque;
using std::pair;
using std::vector;

class noteCmp {
  bool operator()(const sheetNote* a, const sheetNote* b) const;
};

struct chordCmp {
  bool operator()(const pair<int, vector<sheetNote*>>& a, const pair<int, vector<sheetNote*>>& b) const {
    return a.first < b.first;
  }
};

struct flagData {
  int startY;
  int endY;
  int flagType;
  int flagDir;
  int stave;
};

struct sheetChordData {
  int leftWidth;
  int rightWidth;

  vector<flagData> flags;

  int getStemPosition() const { return leftWidth; }
  int getSize() const { return leftWidth + rightWidth; }
};

class sheetMeasure {
 public:
  bool hasStem(int chordNum) const;
  int hasFlag(int chordNum) const;
  int getFlagType(const int noteType) const;

  void buildChordMap(deque<sheetNote>& vecNote);
  void buildFlagMap();

  void setParent(measureController& m) { measure = &m; }

  int getSpacingCount() const;

  vector<sheetChordData> s_chordData;
  vector<pair<int, vector<sheetNote*>>> chords;
  measureController* measure;

 private:
};
