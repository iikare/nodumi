#include <algorithm>
#include <iterator>
#include <raylib.h>
#include "measure.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "wrap.h"

using std::make_move_iterator;

void measureController::clear() {
  // NOTE: not used
  //notes.clear();
  //timeSignatures.clear();
  //keySignatures.clear();
  //currentTime = timeSig();
  //currentKey = keySig();
}

void measureController::buildTickMap() {
  // invariant of absolute value of lowest tick resolution
  int minTick = ctr.getMinTickLen();
  int numPos = tickLength / minTick;

  //logQ(number, ":", currentTime.getTop(), currentTime.getBottom(), "#pos", numPos); 

  vector<int> tmpPos(numPos, tick);
  for (unsigned int i = 0; i < tmpPos.size(); ++i) {
    tmpPos[i] += i * minTick;
  }

  //logQ(formatVector(tmpPos));

  tickMap = set<int>(make_move_iterator(tmpPos.begin()), make_move_iterator(tmpPos.end()));
}

void measureController::addNote(note& note) {
  notes.push_back(&note);

  // below this point: not required for non-sheetmusic usage


  // find available sheet parameters (quantize)
  if (!tickMap.contains(note.tick)) {
    // not a standard position, needs quantization
    int quantTick = *tickMap.lower_bound(note.tick);
    //logQ("nonstandard tickpos with size", note.size, "on measure", note.measure, "rectified to", quantTick);

    displayNotes.push_back({quantTick, &note}); 
  }
  else {
    displayNotes.push_back({note.tick, &note}); 
  }
}
