#include <iostream>
#include <bitset>
#include <cmath>
#include "note.h"
#include "data.h"
#include "log.h"

using std::cerr;
using std::endl;
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

void note::findSheetParameters() {
  int mappedPos = y - MIN_NOTE_IDX;
  //int octave = (9 + mappedPos) / 12;
  int keyIndex = ((9 + mappedPos) % 12);

  // TODO: add getStartingIndex() check

  // 0, 2, 3, 5, 7, 8, 10 are in key
  if (keyIndex == 0 || keyIndex == 2 || keyIndex == 3 || keyIndex == 5 || keyIndex == 7 || keyIndex == 8 || keyIndex == 10) {
    // is part of the key and can be added normally
  //  sheetView.accidentalType = ACC_NONE;
 //   sheetView.keyIndex = (keyIndex + octave * 7) - 9;
  }
  else {
    // need to shift based on key and key index
    // indexes 1, 4, 6, 9, 11
  }

  //TODO: fix getStartingIndex() segfault on files with 0 key sigs
  //cerr << key->getStartingIndex() << " " << (octave * 7 + keyIndex) - 9 << " " << (9 + mappedPos) % 12 << endl;
}
