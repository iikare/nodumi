#include "note.h"

#include <bitset>
#include <cmath>

#include "data.h"
#include "log.h"
#include "midi.h"

void note::findKeyPos(int ks_type) {
  if (ks_type == KEYSIG_NONE) {
    logW(LL_WARN, "note", number, "with empty keysig", ks_type);
  }
  keySig ks(ks_type, false, 0);
  findKeyPos(ks);
}
void note::findKeyPos(const keySig& key) {
  int mappedPos = y - MIN_NOTE_IDX;
  int octave = (9 + mappedPos) / 12;
  int keyStart = key.getIndex();
  // int noteIndex = (12 + ((9 + mappedPos) % 12) - keyStart) % 12;
  int noteIndex = (12 + ((9 + mappedPos) % 12) - keyStart) % 12;

  auto keyParams = staveKeyMap[key.getKey()][noteIndex % 12];

  accType = keyParams.acc;

  sheetY = (octave - 4) * 7 + keyParams.offset + key.getStaveOffset();

  // logQ("[", measure, number, y, "]", keyParams.offset, keyParams.acc, sheetY);
}

void note::findSize(const set<pair<int, int>, tickCmp>& tickSet) {
  auto it = tickSet.lower_bound(make_pair(tickDuration, 0));

  // logQ(tickDuration, "map to notevalue", it->second, "- eighth:", NOTE_8);
  type = it->second;
}

bool note::hasDot() const {
  switch (type) {
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
