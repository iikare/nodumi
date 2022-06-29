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
  if (sheetData.keySignatureMap.size() == 0) {
    return;
  }
  if (idxNote.x >= sheetData.keySignatureMap[sheetData.keySignatureMap.size() - 1].first) {
    idxNote.setKeySig(&(sheetData.keySignatureMap[sheetData.keySignatureMap.size() - 1].second));
    return;
  }
  for (unsigned int i = 0; i < sheetData.keySignatureMap.size(); i++) {
    if (sheetData.keySignatureMap[i].first <= idxNote.x && sheetData.keySignatureMap[i + 1].first > idxNote.x) {
      idxNote.setKeySig(&(sheetData.keySignatureMap[i].second));
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
  tickMap.clear();
  sheetData.reset();

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
      sheetData.addTimeSignature(midifile[0][i].seconds * 500, midifile[0][i].tick,
                                 {(int)midifile[0][i][3], (int)pow(2, (int)midifile[0][i][4]), -1});
    }
    if (midifile[0][i].isKeySignature()) {
      //log3(LL_INFO, "key sig at event", i);
      //cerr << (int)midifile[0][i][1] << " " << (int)midifile[0][i][3] << " " << (int)midifile[0][i][4] <<  endl;
      //cerr << midifile[0][i].seconds * 500 << endl;;
      sheetData.addKeySignature(midifile[0][i].seconds * 500, midifile[0][i].tick, 
                                sheetData.eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4]));
      }
  }

  // link keysigs
  sheetData.linkKeySignatures();

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
  if (sheetData.timeSignatureMap.size() == 0) {
    //logW(LL_WARN, "attempt to load MIDI with no time signatures!");
    sheetData.timeSignatureMap.push_back(make_pair(0,timeSig(4,4,0)));
    //exit(1);
  }

  int cTick = 0;
  timeSig cTimeSig = sheetData.timeSignatureMap[0].second;
  idx = 0;

  for (unsigned int i = 0; i < sheetData.timeSignatureMap.size(); i++) {
    //cerr << sheetData.timeSignatureMap[i].second.top << " " << sheetData.timeSignatureMap[i].second.bottom;
    //cerr << ": " << sheetData.timeSignatureMap[i].first << endl;//<< " " << sheetData.timeSignatureMap[i].second.bottom << endl;
  }
  
  
  measureMap.push_back(measureController(0, 0, cTimeSig.qpm * tpq));
  while (idx < (int)sheetData.timeSignatureMap.size()) {

  //  cerr << cTimeSig.top << " " << cTimeSig.bottom << endl;
    if (idx + 1 != (int)sheetData.timeSignatureMap.size()) {
      cTick += cTimeSig.qpm * tpq;
      if (midifile.getTimeInSeconds(cTick) * 500>= sheetData.timeSignatureMap[idx + 1].first) {
        cTimeSig = sheetData.timeSignatureMap[++idx].second;
      }
      measureMap.push_back(measureController(midifile.getTimeInSeconds(cTick) * 500, cTick, cTimeSig.qpm * tpq));
    }
    else {
      while (cTick < lastTick) {
        cTick += cTimeSig.qpm * tpq;
        
        measureMap.push_back(measureController(midifile.getTimeInSeconds(cTick) * 500, cTick, cTimeSig.qpm * tpq));
      }
      break;
    }
  }
  measureMap.pop_back(); 

  // assign notes, TODO:key signatures to measures
  auto measureStartCmp = [] (pair<int, int> a, pair<int, int> b) { return a.first <= b.first; };
  set<pair<int, int>, decltype(measureStartCmp)> measureStart = {};
  for (int m = 0; auto& measure : measureMap) {
    //logQ(measure.getTick());
    measureStart.insert(make_pair(measure.getTick(), m++));
    measure.notes.clear();
  }

  for (auto& note : notes) {
    auto mIt = measureStart.lower_bound(make_pair(note.tick, 0));
    // measures have 0-based index, but 1-based for rendering
    int noteMeasure = (mIt != measureStart.begin() ? (--mIt)->second : 0);

    //logQ(note.tick, "has closest measure start", noteMeasure);

    note.measure = noteMeasure;

    measureMap[noteMeasure].notes.push_back(&note);
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

