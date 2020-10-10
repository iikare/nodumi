#include <algorithm>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "sheetctr.h"
#include "define.h"

using std::max;

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
    logII(LL_CRIT, "invalid MIDI");
    return;
  }
  
  for (int i = 0; i < 8; i++) {
    tickMap.push_back(tpq * 4 * pow(2, -i));
  }
}

void midi::findMeasure(note& idxNote) {
  // requires a built measure map
  if (measureMap[measureMap.size() - 1].location < idxNote.x) {
   idxNote.measure = measureMap.size() - 1;
   measureMap[measureMap.size() - 1].notes.push_back(&idxNote);
   return;
  }
  for (unsigned int i = 0; i <= measureMap.size(); i++) {
    if (measureMap[i].location < idxNote.x && measureMap[i + 1].location > idxNote.x) {
      idxNote.measure = i;
      measureMap[i].notes.push_back(&idxNote);
      return;
    }
  }
  idxNote.measure = -1;
  return;
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

int midi::findParentMeasure(int measure) {
  if (measure < 0 || measure > static_cast<int>(measureMap.size())) {
    return -1;
  }
  else if (!measure) {
    return 1;
  }
  return measureMap[measure - 1].parentMeasure + 1;
}

void midi::load(string file) {
  MidiFile midifile;
  if (!midifile.read(file.c_str())) {
    logII(LL_WARN, "unable to open MIDI");
    return;
  }

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
    logII(LL_WARN, "zero length file");
    return;
  }

  sort(trackInfo.begin(), trackInfo.end());

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
      sheetData.addTimeSignature(midifile[0][i].seconds * 500, {(int)midifile[0][i][3], (int)pow(2, (int)midifile[0][i][4])});
    }
    if (midifile[0][i].isKeySignature()) {
      //log3(LL_INFO, "key sig at event", i);
      //cerr << (int)midifile[0][i][1] << " " << (int)midifile[0][i][3] << " " << (int)midifile[0][i][4] <<  endl;
      //cerr << midifile[0][i].seconds * 500 << endl;;
      sheetData.addKeySignature(midifile[0][i].seconds * 500, 
                                sheetData.eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4]));
      }
  }

  // build track height map
  for (unsigned int i = 0; i < tracks.size(); i++) {
    trackHeightMap.push_back(make_pair(i, tracks[i].getAverageY()));
  }
  
  sort(trackHeightMap.begin(), trackHeightMap.end(), [](const pair<int,int> &left, const pair<int,int> &right) {
    return left.second < right.second;
  });
 
  // build measure map
  int cTick = 0;
  timeSig cTimeSig = sheetData.timeSignatureMap[0].second;
  idx = 0;

  for (unsigned int i = 0; i < sheetData.timeSignatureMap.size(); i++) {
    cerr << sheetData.timeSignatureMap[i].second.top << " " << sheetData.timeSignatureMap[i].second.bottom;
    cerr << ": " << sheetData.timeSignatureMap[i].first << endl;//<< " " << sheetData.timeSignatureMap[i].second.bottom << endl;
  }
  
  
  measureMap.push_back(0);
  while (idx < (int)sheetData.timeSignatureMap.size()) {

  //  cerr << cTimeSig.top << " " << cTimeSig.bottom << endl;
    if (idx + 1 != (int)sheetData.timeSignatureMap.size()) {
      cTick += cTimeSig.qpm * tpq;
      if (midifile.getTimeInSeconds(cTick) * 500>= sheetData.timeSignatureMap[idx + 1].first) {
        cTimeSig = sheetData.timeSignatureMap[++idx].second;
      }
      measureMap.push_back(midifile.getTimeInSeconds(cTick) * 500);
    }
    else {
      while (cTick < lastTick) {
        cTick += cTimeSig.qpm * tpq;
        
        measureMap.push_back(midifile.getTimeInSeconds(cTick) * 500);
      }
      break;
    }
  }
  measureMap.pop_back(); 

  // assign measures to notes
  for (unsigned int i = 0; i < notes.size(); i++) {
    findMeasure(notes[i]);
  }

  // then find length of measure from notes
  int adjustedLength = 0;
  for (unsigned int i = 0; i < measureMap.size(); i++) {
    measureMap[i].findLength();
    measureMap[i].displayX += adjustedLength;
    adjustedLength += measureMap[i].getLength();

    //cerr << i << ": " << measureMap[i].length << " " << measureMap[i].displayX - measureMap[measureMap[i].parentMeasure].displayX <<  endl;
  }

  // then wrap measures to segments and resize measures to fit
//  cerr << measureMap[measureMap.size() - 1].getDisplayLocation() << endl;
  if (measureMap[measureMap.size() - 1].getDisplayLocation() + measureMap[measureMap.size() - 1].getLength() <
      ctr.getSheetSize()) {
    // handle single page case
    
    double expandRatio = static_cast<double>(ctr.getSheetSize()) / (measureMap[measureMap.size() - 1].getDisplayLocation() + measureMap[measureMap.size() - 1].getLength());
    //cerr << expandRatio << endl;

    for (unsigned int i = 0; i < measureMap.size(); i++) {
      measureMap[i].parentMeasure = 0;
      measureMap[i].displayX *= expandRatio;
      measureMap[i].displayLength = measureMap[i].getLength() * expandRatio;
    }
  }
  else {
    int lastMeasureBreak = 0;
    for (int i = 0; i <= (int)measureMap.size(); i++) {
      if (measureMap[i].getDisplayLocation() + measureMap[i].getLength() - measureMap[lastMeasureBreak].getDisplayLocation() > 
          ctr.getSheetSize() || i == (int)measureMap.size()) {
        
        // TODO: move this
        int extraSpace = ctr.getSheetSize() -
                         (measureMap[i].getDisplayLocation() - measureMap[lastMeasureBreak].getDisplayLocation());
        double expandRatio = 1.0 + static_cast<double>(extraSpace) / ctr.getSheetSize();
        
        int measureLimit = (i == (int)measureMap.size() ? i : i - 2);



        for (int j = lastMeasureBreak; j <= measureLimit; j++) {
          measureMap[j].parentMeasure = lastMeasureBreak;
         
          // TODO: move this
          measureMap[j].displayX *= expandRatio;
          measureMap[j].displayLength = measureMap[j].getLength() * expandRatio;
          
        }
        lastMeasureBreak = i - 1;
         // cerr << expandRatio << endl;
      }
    }
  }

  for (int i = (int)measureMap.size() - 1; i >= 0; i--) {
    measureMap[i].displayX -= measureMap[measureMap[i].parentMeasure].displayX;

    // TODO: move extra space scaling over here
          cerr << i + 1 << ": "
               << measureMap[i].parentMeasure + 1<< " | "
               << measureMap[i].length << " " << measureMap[i].displayX << endl;
  }

  // build line vertex map
  buildLineMap();

  //lastTime = notes[getNoteCount() - 1].x + notes[getNoteCount() - 1].duration;
  //logII(LL_CRIT, (midifile.getFileDurationInTicks()) / (tpq * 4) + 1);
  //logII(LL_CRIT, measureMap.size());
    
}

