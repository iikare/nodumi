#include <algorithm>
#include <raylib.h>
#include "measure.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "wrap.h"

using std::find;
using std::sort;

void measureController::clear() {
  notes.clear();
  timeSignatures.clear();
  keySignatures.clear();
  currentTime = timeSig();
  currentKey = keySig();
}
