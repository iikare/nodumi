#include <algorithm>
#include <raylib.h>
#include "measure.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "wrap.h"

using std::find;
using std::sort;
 

void measureController::findLength() {
  //cerr << timeSignatures.size() << " " << keySignatures.size()<< endl;

  for (unsigned int i = 0; i < timeSignatures.size(); i++) {
    //cerr << timeSignatures[i]->getSize() << endl;
    allEvents.push_back(UMO(timeSignatures[i], timeSignatures[i]->getSize()));
  }
  for (unsigned int i = 0; i < keySignatures.size(); i++) {
    //cerr << keySignatures[i]->getSize() << endl;
    allEvents.push_back(UMO(keySignatures[i], keySignatures[i]->getSize()));
  }
  
  vector<double> pos;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (find(pos.begin(), pos.end(), notes[i]->tick) == pos.end()) {
      pos.push_back(notes[i]->tick);
      allEvents.push_back(UMO(notes[i], 1));
    }
    else {
      int idx = 0;
      for (unsigned int j = 0; j < allEvents.size(); j++) {
        if (allEvents[j].getTick() == notes[i]->tick && allEvents[j].getType() == UMO_NOTE) {
          idx = j;
          break;
        }
      }
      allEvents[idx].addNote(notes[i]);
    }
  }
  length = (getUMOWidth() + 1) * SHEET_NOTEWIDTH;
  //cerr << " " << getUMOWidth() << endl;
  //cerr << allEvents.size() << getUMOEvents() << " " << notes.size() << " " << timeSignatures.size() << " " << keySignatures.size() << endl;
  sort(allEvents.begin(), allEvents.end(), [](const UMO& left, const UMO& right) {
    if (left.getTick() != right.getTick()) {
      return left.getTick() < right.getTick();
    }
    else if (left.getType() == UMO_TIME && right.getType() != UMO_TIME) {
      return true;
    }
    else if (left.getType() != UMO_TIME && right.getType() == UMO_TIME) {
      return false;
    }
    else if (left.getType() == UMO_KEY && right.getType() != UMO_KEY) {
      return true;
    }
    else if (left.getType() != UMO_KEY && right.getType() == UMO_KEY) {
      return false;
    }   
    return true; 

  });
  //logII(LL_CRIT, uniquePositions);
}

void measureController::draw() {
  double cSpaceIdx= 0.35;
  for (unsigned int i = 0; i < allEvents.size(); i++) {
    double relativePosition = (allEvents[i].getTick() - tick) / static_cast<double>(tickLength);
    double absolutePosition = SHEET_LMARGIN + displayX + (expandRatio * cSpaceIdx * SHEET_NOTEWIDTH);

    switch(allEvents[i].getType()) {
      case UMO_NOTE:
        {
          vector<note*>* chord = static_cast<vector<note*>*>(allEvents[i].getRawEvent());
          for (unsigned int j = 0; j < chord->size(); j++) { 
            float noteHeadX = round(absolutePosition);
            float noteHeadY = getSheetY(chord->at(j)->y); 
            DrawTextureEx(ctr.quarter, {noteHeadX, noteHeadY}, 0, 1.0f, {0, 0, 0, 255});
            DrawTextureEx(ctr.flag, {noteHeadX + 10, noteHeadY - 25}, 0, 1.0f, {0, 0, 0, 255});
        
            drawLineEx(noteHeadX + 10, noteHeadY + 4, noteHeadX + 10, noteHeadY - 25, 1.5, ctr.bgDark);
          }
        }
        break;
      case UMO_TIME:
        drawTextEx(ctr.fontMusic, to_string(static_cast<timeSig*>(allEvents[i].getRawEvent())->top),
                   absolutePosition, ctr.barMargin + 19, ctr.bgDark);
        drawTextEx(ctr.fontMusic, to_string(static_cast<timeSig*>(allEvents[i].getRawEvent())->bottom),
                   absolutePosition, ctr.barMargin + 39, ctr.bgDark);
        drawTextEx(ctr.fontMusic, to_string(static_cast<timeSig*>(allEvents[i].getRawEvent())->top),
                   absolutePosition, ctr.barSpacing + ctr.barMargin + 19, ctr.bgDark);
        drawTextEx(ctr.fontMusic, to_string(static_cast<timeSig*>(allEvents[i].getRawEvent())->bottom),
                   absolutePosition, ctr.barSpacing + ctr.barMargin + 39, ctr.bgDark);
        break;
      case UMO_KEY:
        drawTextEx(ctr.fontMusic, to_string(static_cast<keySig*>(allEvents[i].getRawEvent())->getKey()),
                   absolutePosition, ctr.barSpacing + ctr.barMargin + 39, ctr.bgDark);
    }
   
    cSpaceIdx += allEvents[i].getSize();
    //cerr << cSpaceIdx << " " << relativePosition << endl; 
  }
}

int measureController::getUMOWidth() {
  int w = 0;
  for (unsigned int i = 0; i < allEvents.size(); i++) {
    w += allEvents[i].getSize();
  }
  return w;
}

int measureController::getUMOEvents() {
  int ev = 0;
  for (unsigned int i = 0; i < allEvents.size(); i++) {
    ev += allEvents[i].getNumEvents();
  }
  return ev;
}

int measureController::getSheetY(int noteY) {
  int distC = 44 - (noteY - MIN_NOTE_IDX);
  cerr << distC<< endl;
  return ctr.barMargin + ctr.barSpacing / 2 + 5 * distC;
}
