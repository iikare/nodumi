#include <algorithm>
#include <raylib.h>
#include "measure.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "wrap.h"

using std::find;
 

void measureController::findLength() {
  //cerr << timeSignatures.size() << " " << keySignatures.size()<< endl;

  int uniquePositions = 0;
  for (unsigned int i = 0; i < timeSignatures.size(); i++) {
    uniquePositions += timeSignatures[i]->getSize();
  }
  for (unsigned int i = 0; i < keySignatures.size(); i++) {
    uniquePositions += keySignatures[i]->getSize();  
   cerr<<keySignatures[i]->getSize() << endl; 
  }
  
  vector<double> pos;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (find(pos.begin(), pos.end(), notes[i]->tick) == pos.end()) {
      pos.push_back(notes[i]->tick);
      uniquePositions++;
    }
  }
  length = (uniquePositions + 2) * SHEET_NOTEWIDTH;
  //logII(LL_CRIT, uniquePositions);
}

void measureController::draw() {
  cerr << notes.size() << endl;
  for (unsigned int i = 0; i < notes.size(); i++) {
    DrawTextureEx(ctr.quarter, {float(15*i + SHEET_LMARGIN + displayX + 20),i + ctr.barMargin + 15.0f}, 0, 1.0f, {0, 0, 0, 255});
  }
}
