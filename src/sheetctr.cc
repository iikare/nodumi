#include "sheetctr.h"

void sheetController::addTimeSignature(int position, timeSig timeSignature) {
  if (timeSignatureMap.size() != 0 && timeSignatureMap[timeSignatureMap.size()-1].second == timeSignature) {
    return;
  }
  timeSignatureMap.push_back(make_pair(position, timeSignature));
}

void sheetController::addKeySignature(int position, keySig keySignature) {
  if (keySignatureMap.size() != 0 && keySignatureMap[keySignatureMap.size()-1].second == keySignature) {
    return;
  }
  keySignatureMap.push_back(make_pair(position, keySignature));
}

keySig sheetController::eventToKeySignature(int keySigType, bool isMinor) {
  int keyType = KEYSIG_NONE;

  switch(keySigType) {
    case 0:
      keyType = KEYSIG_C;
      break;
    case 1:
      keyType = KEYSIG_G;
      break;
    case 2:
      keyType = KEYSIG_D;
      break;
    case 3:
      keyType = KEYSIG_A;
      break;
    case 4:
      keyType = KEYSIG_E;
      break;
    case 5:
      keyType = KEYSIG_B;
      break;
    case 6:
      keyType = KEYSIG_FSHARP;
      break;
    case 7:
      keyType = KEYSIG_CSHARP;
      break;
    case 255:
      keyType = KEYSIG_F;
      break;
    case 254:
      keyType = KEYSIG_BFLAT;
      break;
    case 253:
      keyType = KEYSIG_EFLAT;
      break;
    case 252:
      keyType = KEYSIG_AFLAT;
      break;
    case 251:
      keyType = KEYSIG_DFLAT;
      break;
    case 250:
      keyType = KEYSIG_GFLAT;
      break;
    case 249:
      keyType = KEYSIG_CFLAT;
      break;
  }


  keySig finalKey = keySig(keyType, isMinor);

  return finalKey;
}

timeSig sheetController::getTimeSignature(int offset) {
  timeSig timeSignature = {0, 1};
  for (unsigned int i = 0; i < timeSignatureMap.size(); i++) {
    if (offset > timeSignatureMap[i].first && offset < timeSignatureMap[i + 1].first) {
      return timeSignatureMap[i].second;
    }
  }
  return timeSignature;
}

keySig sheetController::getKeySignature(int offset) {
  keySig keySignature = {0, 1};
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (offset > keySignatureMap[i].first && offset < keySignatureMap[i + 1].first) {
      return keySignatureMap[i].second;
    }
  }
  return keySignature;
}
