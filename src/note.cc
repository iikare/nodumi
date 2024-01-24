#include "note.h"

#include <bitset>
#include <cmath>

#include "data.h"
#include "log.h"
#include "midi.h"

void note::findKeyPos(const keySig& key) {
  int mappedPos = y - MIN_NOTE_IDX;
  int octave = (9 + mappedPos) / 12;
  int keyStart = key.getIndex();
  int noteIndex = (12 + ((9 + mappedPos) % 12) - keyStart) % 12;

  auto keyParams = staveKeyMap[key.getKey()][noteIndex % 12];

  accType = keyParams.acc;

  sheetY = (octave - 4) * 7 + keyParams.offset + key.getStaveOffset();

  // logQ(keyParams.offset, keyParams.acc, sheetY);
}

void note::findSize(const set<pair<int, int>, tickCmp>& tickSet) {
  auto it = tickSet.lower_bound(make_pair(tickDuration, 0));

  // logQ(tickDuration, "map to notevalue", it->second);
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
