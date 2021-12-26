#include <string>
#include <iostream>
#include "track.h"

using std::to_string;
using std::pair;
using std::cout;
using std::endl;

void trackController::insert(int idx, note* newNote) {
  if (newNote == nullptr) {
    logW(LL_CRIT, "empty note passed to trackController::insert with index", idx);
    return;
  }
  if (tail != nullptr && tail->x > newNote->x) {
    logW(LL_WARN, "mismatched note on track", newNote->track," (", tail->x, " → ", newNote->x, ")");
  }
  if (head == nullptr) {
    head = newNote;
  }
  if (tail == nullptr) {
    tail = newNote;
  }
  else if (tail->x == newNote->x) {
    tail->chordNext = newNote;
    newNote->prev = tail;
  }
  else {
    // find chord root
    note* p = tail;
    while (p->prev != nullptr && p->prev->x == p->x) {
      p = p->prev;
    }
    p->next = newNote;
    newNote->prev = p;
  }
  
  tail = newNote;

  noteIdxMap.insert(pair<int, note*>(idx, newNote));
  noteCount++;
  noteSum+= newNote->y;
}

note* trackController::getNote(int idx) {
  note* noteptr = nullptr;
  try {
    noteptr = noteIdxMap.at(idx);
  }
  catch (const std::out_of_range& e) {
    //logII(LL_INFO, "note not found");
    return nullptr;
  }
  return noteptr;
}

note* trackController::getLastNote() {
  if (noteIdxMap.size() == 0) {
    return nullptr;
  }
  return noteIdxMap[noteIdxMap.size() - 1];
}

void trackController::fixLastNote() {
  if (!getLastNote()) {
    return;
  }
  getLastNote()->isLastOnTrack = true;
}
