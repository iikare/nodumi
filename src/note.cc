#include <iostream>
#include <bitset>
#include <cmath>
#include "note.h"

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
  return !(this->next == nullptr);
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

void note::findSize(int tpq) {
  /* 
   * 0x 00000000 00000000 00000000 00000000
   *                             | ||||||||__ 64th
   *                             | |||||||___ 32nd
   *                             | ||||||____ 16th
   *                             | |||||_____ 8th
   *                             | ||||______ quarter
   *                             | |||_______ half
   *                             | ||________ whole
   *                             | |_________ null
   *                             |___________ counter for whole notes
   */
  size = 0;
  double remainder = 1.05727 * fmod(tickDuration, tpq * 4);
  bool largeNoteFlag = tickDuration > tpq * 4;
  for (int i = 0; i < 7; i++) {
      std::cerr << remainder << " ";
      std::cerr << tickDuration << " ";
    if (round(remainder) == 0) {
      break;
    }
    if (fmod(remainder, tpq * 4 * pow(2, -i)) < remainder) {
      remainder -= tpq * 4 * pow(2, -i);
      size = (1 << (6 - i)) | size;
    }
  }
  if (largeNoteFlag) {
    // deal with very large notes
  }
  std::cerr << "size = " << std::bitset<32>(size) << std::endl;
}
