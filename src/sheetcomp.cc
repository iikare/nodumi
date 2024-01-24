#include "sheetcomp.h"

#include <algorithm>
#include <type_traits>

#include "note.h"
#include "sheetctr.h"

using std::min;

void sheetMeasure::buildChordMap(vector<sheetNote>& vecNote) {
  // intermediate representation of chordmap (needs to be tick-sorted)
  set<pair<int, vector<sheetNote*>>, chordCmp> chordSet;

  for (auto& n : vecNote) {
    auto it = chordSet.find(make_pair(n.displayBegin, vector<sheetNote*>{}));
    if (it != chordSet.end()) {
      // add to chordSet
      auto vec = const_cast<vector<sheetNote*>*>(&(it->second));
      vec->push_back(&n);
    }
    else {
      pair<int, vector<sheetNote*>> newPos =
          make_pair(n.displayBegin, vector<sheetNote*>{&n});
      chordSet.insert(newPos);
    }
  }
  chords = vector<pair<int, vector<sheetNote*>>>(
      make_move_iterator(chordSet.begin()), make_move_iterator(chordSet.end()));
  // chords = vector<pair<int,vector<sheetNote*>>>(chordSet.begin(),
  // chordSet.end());

  s_chordData.resize(chords.size());
}

void sheetMeasure::buildFlagMap() {
  // assign flag positions
  for (int chordNum = 0; const auto& c : chords) {
    flagData cFlag;
    int chn = 0;
    for (unsigned int n = 0; n < c.second.size(); ++n) {
      if (c.second[n]->oriNote->sheetY >
              sheetController::getStaveRenderLimit().first ||
          c.second[n]->oriNote->sheetY <
              sheetController::getStaveRenderLimit().second) {
        continue;
      }
      if (!c.second[n]->visible) {
        continue;
      }
      // ignore oversize notes
      if (c.second[n]->oriNote->type < NOTE_HALF_DOT) {
        continue;
      }

      if (chn == 0) {
        // from bottom, check if flag needs to face up

        if (c.second[n]->oriNote->sheetY <
            sheetController::getFlagLimit(
                getFlagType(c.second[n]->oriNote->type), STAVE_BASS)) {
          cFlag.flagDir = FLAG_UP;  // default due to space constraint
        }
        else {
          // if first note has space for a flag-down position
          cFlag.flagDir = FLAG_DOWN;
          // invert notehead due to down-flag
          c.second[n]->left = false;
        }
        cFlag.startY = c.second[n]->oriNote->sheetY;
        cFlag.endY = cFlag.startY;
        cFlag.flagType = getFlagType(c.second[n]->oriNote->type);
        cFlag.stave = STAVE_BASS;
      }
      else {
        // all other notes
        cFlag.startY = c.second[n]->oriNote->sheetY;
        // take larger of the stem types
        cFlag.flagType =
            min(cFlag.flagType, getFlagType(c.second[n]->oriNote->type));
        if (cFlag.flagDir == FLAG_DOWN) {
          c.second[n]->left = false;
        }
      }

      ++chn;
      if (c.second[n]->stave == STAVE_TREBLE) {
        break;
      }
    }

    s_chordData[chordNum].flags.push_back(cFlag);

    chn = 0;
    for (auto nIt = c.second.rbegin(); nIt != c.second.rend(); ++nIt) {
      if ((*nIt)->oriNote->sheetY >
              sheetController::getStaveRenderLimit().first ||
          (*nIt)->oriNote->sheetY <
              sheetController::getStaveRenderLimit().second) {
        continue;
      }
      if (!(*nIt)->visible) {
        continue;
      }
      // ignore oversize notes
      if ((*nIt)->oriNote->type < NOTE_HALF_DOT) {
        continue;
      }

      if (chn == 0) {
        // from top, check if flag needs to face down

        if ((*nIt)->oriNote->sheetY >
            sheetController::getFlagLimit(getFlagType((*nIt)->oriNote->type),
                                          STAVE_TREBLE)) {
          cFlag.flagDir = FLAG_DOWN;  // default due to space constraint
          // invert notehead due to down-flag
          (*nIt)->left = false;
        }
        else {
          // if first note has space for a flag-up position
          cFlag.flagDir = FLAG_UP;
        }
        cFlag.startY = (*nIt)->oriNote->sheetY;
        cFlag.endY = cFlag.startY;
        cFlag.flagType = getFlagType((*nIt)->oriNote->type);
        cFlag.stave = STAVE_BASS;
      }
      else {
        // all other notes
        cFlag.endY = (*nIt)->oriNote->sheetY;
        // take larger of the stem types
        cFlag.flagType =
            min(cFlag.flagType, getFlagType((*nIt)->oriNote->type));
        if (cFlag.flagDir == FLAG_DOWN) {
          (*nIt)->left = false;
        }
      }

      ++chn;
      if ((*nIt)->stave == STAVE_BASS) {
        break;
      }
    }
    s_chordData[chordNum].flags.push_back(cFlag);

    chordNum++;
  }
}

bool sheetMeasure::hasStem(int chordNum) const {
  for (const auto& chordNote : chords[chordNum].second) {
    // enum is inverted
    if (chordNote->oriNote->type > NOTE_WHOLE) {
      return true;
    }
  }
  return false;
}

int sheetMeasure::hasFlag(int chordNum) const {
  bool hasFlag = false;

  for (const auto& chordNote : chords[chordNum].second) {
    // enum is inverted
    if (chordNote->oriNote->type > NOTE_QUARTER) {
      hasFlag = true;
    }
  }
  if (hasFlag) {
    return FLAG_NONE;
  }

  return false;
}

int sheetMeasure::getSpacingCount() const {
  if (chords.empty()) {
    return 0;
  }
  return chords.size() - 1;
}

int sheetMeasure::getFlagType(const int noteType) const {
  switch (noteType) {
    case NOTE_LARGE:
    case NOTE_WHOLE_DOT:
    case NOTE_WHOLE:
      return FLAGTYPE_NONE;
    case NOTE_HALF_DOT:
    case NOTE_HALF:
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
      return FLAGTYPE_STEM;
    case NOTE_8_DOT:
    case NOTE_8:
      return FLAGTYPE_8;
    case NOTE_16_DOT:
    case NOTE_16:
      return FLAGTYPE_16;
    case NOTE_32:
      return FLAGTYPE_32;
    case NOTE_64:
      return FLAGTYPE_64;
    default:
      logQ("invalid note type:", noteType);
      break;
  }

  return -1;
}
