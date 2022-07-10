#include <algorithm>
#include <string>
#include "timekey.h"
#include "log.h"

using std::max;
using std::to_string;

void keySig::findAccidentalsFromKey() {
  switch (key) {
    case KEYSIG_C:
      accidentals = 0;
      break;
    case KEYSIG_G:
      accidentals = 1;
      break;
    case KEYSIG_D:
      accidentals = 2;
      break;
    case KEYSIG_A:
      accidentals = 3;
      break;
    case KEYSIG_E:
      accidentals = 4;
      break;
    case KEYSIG_B:
      accidentals = 5;
      break;
    case KEYSIG_FSHARP:
      accidentals = 6;
      break;
    case KEYSIG_CSHARP:
      accidentals = 7;
      break;
    case KEYSIG_F:
      accidentals = -1;
      break;
    case KEYSIG_BFLAT:
      accidentals = -2;
      break;
    case KEYSIG_EFLAT:
      accidentals = -3;
      break;
    case KEYSIG_AFLAT:
      accidentals = -4;
      break;
    case KEYSIG_DFLAT:
      accidentals = -5;
      break;
    case KEYSIG_GFLAT:
      accidentals = -6;
      break;
    case KEYSIG_CFLAT:
      accidentals = -7;
      break;
    default:
      accidentals = 10;
  }
}

int keySig::getSize() const {
  return abs(accidentals);
}

void keySig::findStartingIndex() {
  switch (key) {
    case KEYSIG_C:
      startingIndex = 0;
      break;
    case KEYSIG_G:
      startingIndex = 7;
      break;
    case KEYSIG_D:
      startingIndex = 2;
      break;
    case KEYSIG_A:
      startingIndex = 9;
      break;
    case KEYSIG_E:
      startingIndex = 4;
      break;
    case KEYSIG_B:
      startingIndex = 11;
      break;
    case KEYSIG_FSHARP:
      startingIndex = 6;
      break;
    case KEYSIG_CSHARP:
      startingIndex = 1;
      break;
    case KEYSIG_F:
      startingIndex = 5;
      break;
    case KEYSIG_BFLAT:
      startingIndex = 10;
      break;
    case KEYSIG_EFLAT:
      startingIndex = 3;
      break;
    case KEYSIG_AFLAT:
      startingIndex = 8;
      break;
    case KEYSIG_DFLAT:
      startingIndex = 1;
      break;
    case KEYSIG_GFLAT:
      startingIndex = 6;
      break;
    case KEYSIG_CFLAT:
      startingIndex = 11;
      break;
    default:
      startingIndex = -1;
      break;
  }
}
