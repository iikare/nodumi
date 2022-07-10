#include <iostream>
#include <bitset>
#include <cmath>
#include "note.h"
#include "data.h"
#include "midi.h"
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

vector<int> note::getLinePositions(note* now, note* next) {
  vector<int> linePos = {};

  note* pNow = now;
  note* pNext = next;
  bool pushLine = false;

  if (!pNext) {
    if (!(pNow->isLastOnTrack)) {
      logW(LL_WARN, "pNext is nullptr");
    }
    return {};
  }

  auto pushVerts = [&] {
      linePos.push_back(now->number);
      
      linePos.push_back(pNow->x);
      linePos.push_back(pNow->y);
      
      if (!pushLine) {
        linePos.push_back(pNext->x);
        linePos.push_back(pNext->y);
      }
      else {
        linePos.push_back(pNow->x + pNow->duration);
        linePos.push_back(pNow->y);
      }
  };

  // only link spatially near notes
  if (now->x + 2 * now->duration < next->x) {
    pushLine = true;
  }


  if (now->getChordSize() == next->getChordSize()) {
    for (int i = 0; i < now->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
      pNext = pNext->chordNext;
    }
  }
  else if (now->getChordSize() > next->getChordSize()) {
    for (int i = 0; i < next->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
      if (i != next->getChordSize() - 1) {
        pNext = pNext->chordNext;
      }
    }
    for (int i = 0; i < now->getChordSize() - next->getChordSize(); i++) {
      pushVerts();
      pNow = pNow->chordNext;
    }
  }
  else {
    for (int i = 0; i < now->getChordSize(); i++) {
      pushVerts();
      pNext = pNext->chordNext;
      if (i != now->getChordSize() - 1) {
        pNow = pNow->chordNext;
      }
    }
    for (int i = 0; i < next->getChordSize() - now->getChordSize(); i++) {
      pushVerts();
      pNext = pNext->chordNext;
    }
  }

  return linePos;
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

void note::findSize(const set<pair<int,int>, tickCmp>& tickSet)  {
  auto it = tickSet.lower_bound(make_pair(tickDuration, 0));

  //logQ(tickDuration, "map to notevalue", it->second);
  type = it->second;
}
