#include <algorithm>
#include <set>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "sheetctr.h"
#include "define.h"

using std::set;
using std::ref;

int midi::getTempo(int offset) {
  int tempo = 120;
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
  return tempo;
}

void midi::buildLineMap() {
  vector<int> tmpVerts;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (notes[i].isChordRoot()) {
      tmpVerts = getLinePositions(&notes[i], notes[i].getNextChordRoot());
      lineVerts.insert(lineVerts.end(), tmpVerts.begin(), tmpVerts.end());
    }
  }
  
  //logII(LL_CRIT, getNoteCount());
  //logII(LL_CRIT, lineVerts.size());
}

void midi::buildTickMap() {
  if (!tpq) {
    logW(LL_CRIT, "invalid MIDI file");
    return;
  }
  
  for (int i = 0; i < 8; i++) {
    tickMap.push_back(tpq * 4 * pow(2, -i));
  }
}

void midi::findMeasure(note& idxNote) {
  // requires a built measure map
  if (measureMap[measureMap.size() - 1].location <= idxNote.x) {
   idxNote.measure = measureMap.size() - 1;
   measureMap[measureMap.size() - 1].notes.push_back(&idxNote);
   return;
  }
  for (unsigned int i = 0; i <= measureMap.size(); i++) {
    if (measureMap[i].location <= idxNote.x && measureMap[i + 1].location > idxNote.x) {
      idxNote.measure = i;
      measureMap[i].notes.push_back(&idxNote);
      return;
    }
  }
  logQ(idxNote.number);
  idxNote.measure = -1;
  return;
}

void midi::findKeySig(note& idxNote) {
  if (keySignatureMap.size() == 0) {
    return;
  }
  if (idxNote.x >= keySignatureMap[keySignatureMap.size() - 1].first) {
    idxNote.setKeySig(&(keySignatureMap[keySignatureMap.size() - 1].second));
    return;
  }
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (keySignatureMap[i].first <= idxNote.x && keySignatureMap[i + 1].first > idxNote.x) {
      idxNote.setKeySig(&(keySignatureMap[i].second));
      return;
    }
  }
}

int midi::findMeasure(int offset) {
  if (!offset ) { return 0; }
  if (measureMap[measureMap.size() - 1].location < offset) {
    return measureMap.size();
  } 
  // requires a built measure map
  for (unsigned int i = 0; i < measureMap.size(); i++) {
    if (measureMap[i].location < offset && measureMap[i + 1].location >= offset) {
      return i + 1;
    }
  }
  return -1;
}

void midi::addTimeSignature(int position, int tick, timeSig timeSignature) {
  if (timeSignatureMap.size() != 0 && timeSignatureMap[timeSignatureMap.size()-1].second == timeSignature) {
    return;
  }
  timeSignature.setTick(tick);
  timeSignatureMap.push_back(make_pair(position, timeSignature));
}

void midi::addKeySignature(int position, int tick, keySig keySignature) {
  if (keySignatureMap.size() != 0 && keySignatureMap[keySignatureMap.size()-1].second == keySignature) {
    return;
  }
  keySignature.setTick(tick);
  keySignatureMap.push_back(make_pair(position, keySignature));
}

keySig midi::eventToKeySignature(int keySigType, bool isMinor) {
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


  keySig finalKey = keySig(keyType, isMinor, -1);

  return finalKey;
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
    keySignatureMap[i].second.setPrev(&keySignatureMap[i].second);
  }
}

void midi::load(string file, stringstream& buf) {
  MidiFile midifile;
  if (!midifile.read(file.c_str())) {
    logW(LL_WARN, "unable to open MIDI");
    return;
  }

  // file is known to work
  ifstream midiData(file);
  buf.str("");
  buf.clear();

  buf << midiData.rdbuf();
  midiData.close();




  notes.clear();
  tempoMap.clear();
  tracks.clear();
  trackHeightMap.clear();
  lineVerts.clear();
  measureMap.clear();
  timeSignatureMap.clear();
  keySignatureMap.clear();
  tickMap.clear();

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

  buildTickMap();

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

        notes[idx].findSize(tickMap);

        //cerr << midifile.getTimeInSeconds(notes[idx].tick) << " " << midifile[i][j].seconds << endl;

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
      addTimeSignature(midifile[0][i].seconds * 500, midifile[0][i].tick,
                                 {(int)midifile[0][i][3], (int)pow(2, (int)midifile[0][i][4]), -1});
    }
    if (midifile[0][i].isKeySignature()) {
      //log3(LL_INFO, "key sig at event", i);
      //cerr << (int)midifile[0][i][1] << " " << (int)midifile[0][i][3] << " " << (int)midifile[0][i][4] <<  endl;
      //cerr << midifile[0][i].seconds * 500 << endl;;
      addKeySignature(midifile[0][i].seconds * 500, midifile[0][i].tick, 
                                eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4]));
      }
  }

  // link keysigs
  linkKeySignatures();

  for (unsigned int i = 0; i < tracks.size(); i++) {
    // assign chord to last note of each track
    tracks[i].fixLastNote();

    // build track height map
    trackHeightMap.push_back(make_pair(i, tracks[i].getAverageY()));
  }
  
  sort(trackHeightMap.begin(), trackHeightMap.end(), [](const pair<int, double> &left, const pair<int, double> &right) {
    return left.second < right.second;
  });
 
  // build measure map
  if (timeSignatureMap.size() == 0) {
    //logW(LL_WARN, "attempt to load MIDI with no time signatures!");
    timeSignatureMap.push_back(make_pair(0,timeSig(4,4,0)));
    //exit(1);
  }

  int cTick = 0;
  timeSig cTimeSig = timeSignatureMap[0].second;
  idx = 0;

  for (unsigned int i = 0; i < timeSignatureMap.size(); i++) {
    //cerr << timeSignatureMap[i].second.top << " " << timeSignatureMap[i].second.bottom;
    //cerr << ": " << timeSignatureMap[i].first << endl;//<< " " << timeSignatureMap[i].second.bottom << endl;
  }
  
  
  measureMap.push_back(measureController(0, 0, cTimeSig.getQPM() * tpq));
  while (idx < (int)timeSignatureMap.size()) {

  //  cerr << cTimeSig.top << " " << cTimeSig.bottom << endl;
    if (idx + 1 != (int)timeSignatureMap.size()) {
      cTick += cTimeSig.getQPM() * tpq;
      if (midifile.getTimeInSeconds(cTick) * 500>= timeSignatureMap[idx + 1].first) {
        cTimeSig = timeSignatureMap[++idx].second;
      }
      measureMap.push_back(measureController(midifile.getTimeInSeconds(cTick) * 500, cTick, cTimeSig.getQPM() * tpq));
    }
    else {
      while (cTick < lastTick) {
        cTick += cTimeSig.getQPM() * tpq;
        
        measureMap.push_back(measureController(midifile.getTimeInSeconds(cTick) * 500, cTick, cTimeSig.getQPM() * tpq));
      }
      break;
    }
  }
  measureMap.pop_back(); 

  // assign notes, TODO:time/key signatures to measures
  auto itemStartCmp = [] (pair<int, int> a, pair<int, int> b) { return a.first <= b.first; };
  set<pair<int, int>, decltype(itemStartCmp)> itemStart = {};
  for (int m = 0; auto& measure : measureMap) {
    //logQ(measure.getTick());
    itemStart.insert(make_pair(measure.getTick(), m++));
    measure.notes.clear();
    measure.timeSignatures.clear();
    measure.keySignatures.clear();
  }

  for (auto& note : notes) {
    auto mIt = itemStart.lower_bound(make_pair(note.tick, 0));
    // measures have 0-based index, but 1-based for rendering
    int noteMeasure = (mIt != itemStart.begin() ? (--mIt)->second : 0);

    //logQ(note.tick, "has closest measure start", noteMeasure);

    note.measure = noteMeasure;

    measureMap[noteMeasure].notes.push_back(&note);
  }


  for (auto& ts : timeSignatureMap) {
    auto mIt = itemStart.lower_bound(make_pair(ts.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int tsMeasure = (mIt != itemStart.begin() ? (--mIt)->second : 0);

    logQ("ts", ts.second.getTick(), "has closest measure start", 1+tsMeasure);

    ts.second.setMeasure(tsMeasure);

    measureMap[tsMeasure].timeSignatures.push_back(&ts.second);
  }

  
  for (auto& ks : keySignatureMap) {
    auto mIt = itemStart.lower_bound(make_pair(ks.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int ksMeasure = (mIt != itemStart.begin() ? (--mIt)->second : 0);

    logQ("ks", ks.second.getTick(), "has closest measure start", 1+ksMeasure);

    ks.second.setMeasure(ksMeasure);

    measureMap[ksMeasure].keySignatures.push_back(&ks.second);
  }
  


  //for (int m = 0; auto& measure : measureMap) {
    //logQ(measure.notes.size(), "notes in measure", 1+m++);
  //}

  // build line vertex map
  buildLineMap();

  //lastTime = notes[getNoteCount() - 1].x + notes[getNoteCount() - 1].duration;
  //logII(LL_CRIT, (midifile.getFileDurationInTicks()) / (tpq * 4) + 1);
  //logII(LL_CRIT, measureMap.size());
    
}

