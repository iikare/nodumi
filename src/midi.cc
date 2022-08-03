#include <algorithm>
#include <set>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "sheetctr.h"
#include "define.h"

using std::set;
using std::ref;
using std::move;

int midi::getTempo(int offset) {
  if (tempoMap.size() != 0 && offset == 0) {
    return tempoMap[0].second;
  }
  if (offset > tempoMap[tempoMap.size() - 1].first) {
    return tempoMap[tempoMap.size() - 1].second;
  }
  for (unsigned int i = 0; i < tempoMap.size(); i++) {
    if (offset > tempoMap[i].first && offset < tempoMap[i + 1].first) {
      return tempoMap[i].second;
    }
  }
  return 120;
}

void midi::buildLineMap() {
  vector<int> tmpVerts;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (notes[i].isChordRoot()) {
      tmpVerts = notes[i].getLinePositions(&notes[i], notes[i].getNextChordRoot());
      lineVerts.insert(lineVerts.end(), tmpVerts.begin(), tmpVerts.end());
    }
  }
  
  //logII(LL_CRIT, getNoteCount());
  //logII(LL_CRIT, lineVerts.size());
}

void midi::buildTickSet() {
  if (!tpq) {
    logW(LL_CRIT, "MIDI lacks TPQ marker");
    return;
  }

  //logQ("real TPQ:", tpq);

  for (int pos = 0; pos < tickNoteTransformLen; ++pos) {
    //logQ(pos, "maps to TICK VALUE", tpq*tickNoteTransform[pos]); 
    tickSet.insert(make_pair(tpq*tickNoteTransform[pos], pos));
  }


}

int midi::findMeasure(int offset) {
  if (!offset ) { return 0; }
  if (measureMap[measureMap.size() - 1].getLocation() < offset) {
    return measureMap.size();
  } 
  // requires a built measure map
  for (unsigned int i = 0; i < measureMap.size(); i++) {
    if (measureMap[i].getLocation() < offset && measureMap[i + 1].getLocation() >= offset) {
      return i + 1;
    }
  }
  return -1;
}

void midi::addTimeSignature(int position, const timeSig& timeSignature) {
  if (timeSignatureMap.size() != 0 && timeSignatureMap[timeSignatureMap.size()-1].second == timeSignature) {
    return;
  }
  timeSignatureMap.push_back(make_pair(position, timeSignature));
}

void midi::addKeySignature(int position, const keySig& keySignature) {
  if (keySignatureMap.size() != 0 && keySignatureMap[keySignatureMap.size()-1].second == keySignature) {
    return;
  }
  keySignatureMap.push_back(make_pair(position, keySignature));
}

keySig midi::eventToKeySignature(int keySigType, bool isMinor, int tick) {
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


  return keySig(keyType, isMinor, tick);
}

timeSig midi::getTimeSignature(int offset) {
  timeSig timeSignature = {0, -1, 1};
  for (unsigned int i = 0; i < timeSignatureMap.size(); i++) {
    if (offset > timeSignatureMap[i].first && offset < timeSignatureMap[i + 1].first) {
      return timeSignatureMap[i].second;
    }
  }
  return timeSignature;
}

keySig midi::getKeySignature(int offset) {
  keySig keySignature = {0, 1, -1};
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (offset > keySignatureMap[i].first && offset < keySignatureMap[i + 1].first) {
      return keySignatureMap[i].second;
    }
  }
  return keySignature;
}



void midi::linkKeySignatures() {
  if (keySignatureMap.size() == 0) {
    return;
  }
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (i == 0) {
      continue;
    }
    keySignatureMap[i].second.setPrev(&keySignatureMap[i-1].second);
  }
}

void midi::load(string file, stringstream& buf) {
  MidiFile midifile;
  if (!midifile.read(file.c_str())) {
    logW(LL_WARN, "unable to open MIDI");
    //return;
  }

  // file is known to work
  ifstream midiData(file);
  buf.str("");
  buf.clear();

  buf << midiData.rdbuf();
  midiData.close();

  load(buf);

}
  
void midi::load(stringstream& buf) {
  
  MidiFile midifile(buf);

  notes.clear();
  tempoMap.clear();
  tracks.clear();
  trackHeightMap.clear();
  lineVerts.clear();
  measureMap.clear();
  timeSignatureMap.clear();
  keySignatureMap.clear();

  tickSet.clear();
  itemStartSet.clear();

  noteCount = 0;
  trackCount = 0;
  tpq = 0;

  midifile.linkNotePairs();
 
  midifile.doTimeAnalysis();
  
  trackCount = midifile.getTrackCount();
  tracks.resize(trackCount);

  tpq = midifile.getTicksPerQuarterNote();
  lastTime = midifile.getFileDurationInSeconds() * 500;
  lastTick = midifile.getFileDurationInTicks();

  buildTickSet();

  vector<pair<double, int>> trackInfo;

  for (int i = 0; i < trackCount; i++) {
    bool once = true;
    for (int j = 0; j < midifile.getEventCount(i); j++) {
      if (midifile[i][j].isNoteOn()) {
        if (once) {
          once = false;
          trackInfo.push_back(make_pair(midifile[i][j].seconds * 500, i));
        }
        noteCount++;
      }
    }
  }

  if (noteCount == 0) {
    logW(LL_WARN, "zero length MIDI file");
    return;
  }

  //sort(trackInfo.begin(), trackInfo.end());

  notes.resize(noteCount);
  int idx = 0;

  for (unsigned int track = 0; track < trackInfo.size(); track++) {
    int i = trackInfo[track].second;
    for (int j = 0; j < midifile.getEventCount(i); j++) {
      if (midifile[i][j].isNoteOn()) {
        notes[idx].number = idx;
        notes[idx].tick = midifile[i][j].tick;
        notes[idx].tickDuration = midifile[i][j].getTickDuration();
        notes[idx].track = i;
        notes[idx].duration = midifile[i][j].getDurationInSeconds() * 500;
        notes[idx].x  = midifile[i][j].seconds * 500;
        notes[idx].y = midifile[i][j].getKeyNumber();
        notes[idx].velocity = midifile[i][j][2];

        //cerr << midifile.getTimeInSeconds(notes[idx].tick) << " " << midifile[i][j].seconds << endl;
        
        notes[idx].findSize(tickSet);


        tracks.at(notes[idx].track).insert(idx, &notes.at(idx));

        idx++;
      }
    }
  }

  midifile.joinTracks();
  midifile.sortTracks();
  
  for (int i = 0; i < midifile.getEventCount(0); i++) {

    if (midifile[0][i].isTempo()) {
      tempoMap.push_back(make_pair(midifile[0][i].seconds * 500, midifile[0][i].getTempoBPM()));
    }
    if (midifile[0][i].isTimeSignature()) {
      //log3(LL_INFO, "time sig at event", j);
      //cerr << (int)midifile[0][i][3] << " " << pow(2, (int) midifile[0][i][4])<< endl;
      addTimeSignature(midifile[0][i].seconds * 500,
                                 {(int)midifile[0][i][3], (int)pow(2, (int)midifile[0][i][4]), midifile[0][i].tick});
    }
    if (midifile[0][i].isKeySignature()) {
      //log3(LL_INFO, "key sig at event", i);
      //cerr << (int)midifile[0][i][1] << " " << (int)midifile[0][i][3] << " " << (int)midifile[0][i][4] <<  endl;
      //cerr << midifile[0][i].seconds * 500 << endl;;
      keySig tmpKS = eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4], midifile[0][i].tick);
      addKeySignature(midifile[0][i].seconds * 500, tmpKS);
      //logQ("type, tick", (int)midifile[0][i][3], midifile[0][i].tick);
      //auto a = eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4], midifile[0][i].tick);
      //logQ("e2ks, tick", a.getAcc(), a.getTick());
    }
  }

  // link keysigs with left-adjacents
  linkKeySignatures();

  for (unsigned int i = 0; i < tracks.size(); i++) {
    // assign chord to last note of each track
    tracks[i].fixLastNote();

    // build track height map
    trackHeightMap.push_back(make_pair(i, tracks[i].getAverageY()));
  }
  
  sort(trackHeightMap.begin(), trackHeightMap.end(), [](const pair<int, double>& left, const pair<int, double>& right) {
    return left.second < right.second;
  });
 
  // build measure map
  if (timeSignatureMap.size() == 0) {
    logW(LL_INFO, "no time signatures detected, adding default time signature");
    timeSignatureMap.push_back(make_pair(0,timeSig(4,4,0)));
  }
  if (keySignatureMap.size() == 0) {
    logW(LL_INFO, "no key signatures detected, adding default key signature");
    keySignatureMap.push_back(make_pair(0,keySig(KEYSIG_C,0,0)));
  }

  int cTick = 0;
  timeSig cTimeSig = timeSignatureMap[0].second;
  keySig cKeySig = keySignatureMap[0].second;
  idx = 0;
  int idxK = 0;


  int measureNum = 1;

  measureMap.push_back(measureController(measureNum++, 0, 0, cTimeSig.getQPM() * tpq, cTimeSig, cKeySig));
  while (cTick < lastTick) {
    cTick += cTimeSig.getQPM() * tpq;

    if (idxK + 1 < static_cast<int>(keySignatureMap.size())) {
      if (cTick >= keySignatureMap[idxK + 1].second.getTick()) {
        cKeySig = keySignatureMap[++idxK].second;
      }
    }

    if (idx + 1 < static_cast<int>(timeSignatureMap.size())) {
      if (cTick >= timeSignatureMap[idx + 1].second.getTick()) {
        cTimeSig = timeSignatureMap[++idx].second;
      }
    }
    //logQ(measureNum, "to",cKeySig.getAcc());
    measureMap.push_back(measureController(measureNum++, midifile.getTimeInSeconds(cTick) * 500, cTick, 
                                           cTimeSig.getQPM() * tpq, cTimeSig, cKeySig));
  }
  measureMap.pop_back(); 

  //auto itemStartCmp = [] (pair<int, int> a, pair<int, int> b) { return a.first <= b.first; };
  //set<pair<int, int>, decltype(itemStartCmp)> itemStartSet = {};
  for (int m = 0; auto& measure : measureMap) {
    //logQ(measure.getTick());
    itemStartSet.insert(make_pair(measure.getTick(), m++));
    //measure.clear();
  }

  for (auto& note : notes) {
    auto mIt = itemStartSet.lower_bound(make_pair(note.tick, 0));
    // measures have 0-based index, but 1-based for rendering
    int noteMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    //logQ(note.tick, "has closest measure start", noteMeasure);

    note.measure = noteMeasure;

    measureMap[noteMeasure].addNote(note);

  }


  for (auto& ts : timeSignatureMap) {
    auto mIt = itemStartSet.lower_bound(make_pair(ts.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int tsMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    //logQ("ts", ts.second.getTick(), "has closest measure start", 1+tsMeasure);
    //logQ("ts", ts.second.getTick(), "has value", ts.second.getTop(), ts.second.getBottom());

    ts.second.setMeasure(tsMeasure);

    measureMap[tsMeasure].timeSignatures.push_back(ts.second);
  }

  
  for (auto& ks : keySignatureMap) {
    auto mIt = itemStartSet.lower_bound(make_pair(ks.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int ksMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    //logQ("ks", ks.second.getAcc(),"@", ks.second.getTick(), "has closest measure start", 1+ksMeasure);

    ks.second.setMeasure(ksMeasure);

    measureMap[ksMeasure].keySignatures.push_back(ks.second);
  }

  // create sheet music position data
  for (/*int z = 0;*/ auto& measure : measureMap) {


    //logQ("measure",z+1,"at tick",measure.getTick());
    //if (measure.keySignatures.size() > 0) {
      //logQ("measure",z+1,"has INSIDE",measure.keySignatures[0].getAcc(), measure.keySignatures[0].getTick());
    //}
    //logQ("measure",z+1,"has keysig",measure.currentKey.getAcc());
    //logQ("measure",z+1,"has timesig",measure.currentTime.getTop(), measure.currentTime.getBottom());

    //z++;

    for (auto& note : measure.notes) {

      // map note position to key (only if on/off signature)

      note->findKeyPos(measure.currentKey);

    }

    sheetData.disectMeasure(measure);
  }

  sheetData.findSheetPages();
  //for (int m = 0; auto& measure : measureMap) {
    //logQ(measure.notes.size(), "notes in measure", 1+m++);
  //}

  // build line vertex map
  buildLineMap();

  //lastTime = notes[getNoteCount() - 1].x + notes[getNoteCount() - 1].duration;
  //logII(LL_CRIT, (midifile.getFileDurationInTicks()) / (tpq * 4) + 1);
  //logII(LL_CRIT, measureMap.size());
  
  //logQ("total ks", keySignatureMap.size());
}

