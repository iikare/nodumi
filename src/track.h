#pragma once

#include <map>
#include "note.h"
#include "log.h"

using std::map;

class trackController {
  public:
    trackController() {
      noteIdxMap = {};
      head = nullptr;
      tail = nullptr;
      noteCount = 0;
      noteSum = 0;
    }

    void insert(int idx, note* newNote);
    void fixLastNote();
    note* getNote(int idx);
    int getNoteCount() { return noteCount; }
    double getAverageY() { return (double)noteSum/noteCount; }

  private:
    note* getLastNote();
    
    map<int, note*> noteIdxMap;
    note* head;
    note* tail;
    int noteCount;
    int noteSum;
    
};
