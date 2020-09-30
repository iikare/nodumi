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
    }

    void insert(int idx, note* newNote);
    note* getNote(int idx);
    int getNoteCount() { return noteCount; }

  private:
    map<int, note*> noteIdxMap;
    note* head;
    note* tail;
    int noteCount;
    
};
