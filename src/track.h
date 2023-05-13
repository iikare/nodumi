#pragma once

#include <map>
#include <vector>
#include "chord.h"
#include "line.h"
#include "note.h"
#include "log.h"

using std::vector;
using std::map;

class trackController {
  public:
    trackController() {
      n_vec = nullptr;;
      note_idx = {};
      chords = {};
      noteIdxMap = {};
      head = nullptr;
      tail = nullptr;
      noteCount = 0;
      noteSum = 0;
    }

    void setNoteVector(vector<note>* vec) { n_vec = vec; };
    void insert(int idx, note* newNote);
    void insert(unsigned int n);
    void reset();
    void fixLastNote();
    int getNoteCount() const { return noteCount; }
    double getAverageY() const { return (double)noteSum/noteCount; }

    void buildChordMap();
    void buildLineMap();

    friend class midi;

  private:
    note* getLastNote();

    void buildLine(unsigned int l, unsigned int r);
  
    vector<note>* n_vec;
    vector<unsigned int> note_idx;
    vector<lineData> lines;
    vector<chordData> chords;
    map<int, note*> noteIdxMap;
    note* head;
    note* tail;
    int noteCount;
    int noteSum;
    
};
