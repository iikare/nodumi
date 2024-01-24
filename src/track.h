#pragma once

#include <map>
#include <vector>

#include "chord.h"
#include "line.h"
#include "log.h"
#include "note.h"

using std::vector;

class trackController {
 public:
  trackController() {
    n_vec = nullptr;
    note_idx = {};
    chords = {};
    noteCount = 0;
    noteSum = 0;
  }

  void setNoteVector(vector<note>* vec) { n_vec = vec; };
  void insert(unsigned int n);
  void reset();
  int getNoteCount() const { return noteCount; }
  double getAverageY() const {
    return static_cast<double>(noteSum) / noteCount;
  }

  void buildChordMap();
  void buildLineMap();

  friend class midi;

 private:
  void buildLine(unsigned int l, unsigned int r);

  vector<note>* n_vec;
  vector<unsigned int> note_idx;
  vector<lineData> lines;
  vector<chordData> chords;
  int noteCount;
  int noteSum;
};
