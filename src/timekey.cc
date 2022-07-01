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
      accidentals = 0;
  }
}

int keySig::getSize() const {
  return abs(accidentals);
}

