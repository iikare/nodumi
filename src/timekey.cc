#include <algorithm>
#include <string>
#include "timekey.h"
#include "define.h"
#include "log.h"

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

// stave offset from neutral C (in range 0-6)
int keySig::getStaveOffset() const {
  switch (key) {
    case KEYSIG_C:
      return 0;
    case KEYSIG_G:
      return 4;
    case KEYSIG_D:
      return 1;
    case KEYSIG_A:
      return 5;
    case KEYSIG_E:
      return 2;
    case KEYSIG_B:
      return 6;
    case KEYSIG_FSHARP:
      return 3;
    case KEYSIG_CSHARP:
      return 0;
    case KEYSIG_F:
      return 3;
    case KEYSIG_BFLAT:
      return 6;
    case KEYSIG_EFLAT:
      return 2;
    case KEYSIG_AFLAT:
      return 5;
    case KEYSIG_DFLAT:
      return 1;
    case KEYSIG_GFLAT:
      return 4;
    case KEYSIG_CFLAT:
      return 0;
    default:
      return 0;
  }
}

string keySig::getLabel() const {
  // NOTE: "#" used as replacement for sharp,
  //     : "b" for flat, replaced with real
  //     : symbol during rendering
  string label = "";
  switch (key) {
    case KEYSIG_C:
      label = "C ";
			break;
    case KEYSIG_G:
      label = "G ";
			break;
    case KEYSIG_D:
      label = "D ";
			break;
    case KEYSIG_A:
      label = "A ";
			break;
    case KEYSIG_E:
      label = "E ";
			break;
    case KEYSIG_B:
      label = "B ";
			break;
    case KEYSIG_FSHARP:
      label = "F# ";
			break;
    case KEYSIG_CSHARP:
      label = "C# ";
			break;
    case KEYSIG_F:
      label = "F ";
			break;
    case KEYSIG_BFLAT:
      label = "Bb ";
			break;
    case KEYSIG_EFLAT:
      label = "Eb ";
			break;
    case KEYSIG_AFLAT:
      label = "Ab ";
			break;
    case KEYSIG_DFLAT:
      label = "Db ";
			break;
    case KEYSIG_GFLAT:
      label = "Gb ";
			break;
    case KEYSIG_CFLAT:
      label = "Cb ";
			break;
    default:
       return "undefined";
  }
  return label += ctr.text.getString("GET_LABEL_MAJOR");
}
