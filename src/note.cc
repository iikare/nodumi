#include <bitset>
#include <cmath>
#include "note.h"
#include "data.h"
#include "midi.h"
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

void note::findKeyPos(const keySig& key) {
  int mappedPos = y - MIN_NOTE_IDX;
  int octave = (9 + mappedPos) / 12;
  int keyStart = key.getIndex();
  int noteIndex = (12 + ((9 + mappedPos) % 12) - keyStart) % 12;


  auto keyParams = staveKeyMap[key.getKey()][noteIndex % 12]; 

  accType = keyParams.acc;

  sheetY = (octave - 4) * 7 + keyParams.offset + key.getStaveOffset();
  
  //logQ(keyParams.offset, keyParams.acc, sheetY);
}

void note::findSize(const set<pair<int,int>, tickCmp>& tickSet)  {
  auto it = tickSet.lower_bound(make_pair(tickDuration, 0));

  //logQ(tickDuration, "map to notevalue", it->second);
  type = it->second;
}

bool note::hasDot() const {
  switch(type) {
    case NOTE_WHOLE_DOT:
    case NOTE_HALF_DOT:
    case NOTE_QUARTER_DOT:
    case NOTE_8_DOT:
    case NOTE_16_DOT:
      return true;
    default:
      return false;
  }
}
