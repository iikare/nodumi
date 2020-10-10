#include <algorithm>
#include "measure.h"
#include "data.h"
#include "log.h"

using std::find;
 

void measureController::findLength() {
  int uniquePositions = 0;
  vector<double> pos;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (find(pos.begin(), pos.end(), notes[i]->tick) == pos.end()) {
      pos.push_back(notes[i]->tick);
      uniquePositions++;
    }
  }
  length = (uniquePositions + 2) * SHEET_NOTEWIDTH;
  logII(LL_CRIT, uniquePositions);
}
