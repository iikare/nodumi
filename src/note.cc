#include <iostream>
#include <bitset>
#include <cmath>
#include "note.h"
#include "data.h"
#include "log.h"

using std::pow;

note* note::getNextNote() {
  if (next != nullptr) {
    if (chordNext == nullptr) {
      return next;
    }
    else {
      return chordNext;
    }
  }
  else {
    note* p = this;
    return p;
    while (p->chordNext != nullptr) {
      p = p->prev;
    }
    return p;
  }
}

note* note::getNextChordRoot() {
  note* p = this;
  // case 1: note is chord root
  if (isChordRoot()) {
    if (next != nullptr) {
      return next;
    }
  }
  // case 2: note is chord member
  else {
    while (!isChordRoot()) {
      if (p->prev != nullptr) {
        p = p->prev;
      }
      else {
        return this;
      }
    }
    return p->next;
  }
  return nullptr;
}

bool note::isChordRoot() {
  return !(this->next == nullptr) || isLastOnTrack;
}

int note::getChordSize() {
  if (!isChordRoot()) {
    return 0;
  }
  else {
    note* p = this;
    int count = 1;
    while (p->chordNext != nullptr) {
      count++;
      p = p->chordNext;
    }
    return count;
  }
}

void note::findSize(vector<int>& noteChart) {
  if (tickDuration > noteChart[0]) {
    size = NOTE_LARGE;
  }
  else {
    for (unsigned int i = 0; i < noteChart.size(); i++) {
      if (noteChart[i] > tickDuration && noteChart[i + 1] < tickDuration) {
        size = i + 1; // map to note enum 
      }  
    }
    if (tickDuration < noteChart[noteChart.size() - 1]) {
      size = NOTE_128;
    }
  }
}

void note::findSheetY() {
  int mappedPos = y - MIN_NOTE_IDX;
  logII(LL_CRIT, mappedPos);  
}
