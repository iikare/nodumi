#include <algorithm>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "sheetctr.h"

using std::max;

int midi::getTempo(int offset) {
  int tempo = 120;
  for (unsigned int i = 0; i < tempoMap.size(); i++) {
    if (offset > tempoMap[i].first && offset < tempoMap[i + 1].first) {
      return tempoMap[i].second;
    }
  }
  return tempo;
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
  noteCount = 0;
  trackCount = 0;

  midifile.linkNotePairs();
 
  midifile.doTimeAnalysis();
  
  trackCount = midifile.getTrackCount();
  tracks.resize(trackCount);

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
        notes[idx].track = i;
        notes[idx].duration = midifile[i][j].getDurationInSeconds() * 500;
        notes[idx].x  = midifile[i][j].seconds * 500;
        notes[idx].y = midifile[i][j].getKeyNumber();
        notes[idx].velocity = midifile[i][j][2];

        lastTick = max(lastTick, notes[idx].x + notes[idx].duration);

        tracks.at(notes[idx].track).insert(idx, &notes.at(idx));

        idx++;
      }
    }
  }

  idx = 0;
 
  midifile.joinTracks();
  midifile.sortTracks();
  
  int lastIdx = 0;

  for (int i = 0; i < midifile.getEventCount(0); i++) {

    if (midifile[0][i].isTempo()) {
      tempoMap.push_back(make_pair(notes[lastIdx].x, midifile[0][i].getTempoBPM()));
    }
      if (midifile[0][i].isTimeSignature()) {
        //log3(LL_INFO, "time sig at event", j);
        //cerr << (int)midifile[0][i][3] << " " << pow(2, (int) midifile[0][i][4])<< endl;
        sheetData.addTimeSignature(notes[idx].x, {(int)midifile[0][i][3], (int)midifile[0][i][4]});
      }
      if (midifile[0][i].isKeySignature()) {
        //log3(LL_INFO, "key sig at event", i);
        //cerr << (int)midifile[0][i][1] << " " << (int)midifile[0][i][3] << " " << (int)midifile[0][i][4] <<  endl;
        sheetData.addKeySignature(notes[idx].x, sheetData.eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4]));
      }

    if (midifile[0][i].isNoteOn()) {
      lastIdx = idx;
      idx++;
    }
  }

  for (unsigned int i = 0; i < tracks.size(); i++) {
    trackHeightMap.push_back(make_pair(i, tracks[i].getAverageY()));
  }
  
  sort(trackHeightMap.begin(), trackHeightMap.end(), [](const pair<int,int> &left, const pair<int,int> &right) {
    return left.second < right.second;
  });
  
  lastTick = notes[getNoteCount() - 1].x + notes[getNoteCount() - 1].duration;
    
  buildLineMap();
}

void midi::buildLineMap() {
  vector<int> tmpVerts;
  for (unsigned int i = 0; i < notes.size(); i++) {
    if (notes[i].isChordRoot()) {
      tmpVerts = getLinePositions(&notes[i], notes[i].getNextChordRoot());
      lineVerts.insert(lineVerts.end(), tmpVerts.begin(), tmpVerts.end());
    }
  }
  for (unsigned int i = 0; i < lineVerts.size(); i += 5) {
    if (lineVerts[i] > getNoteCount()) {
      logII(LL_CRIT, "lineVerts v. noteCount: " + to_string(lineVerts[i]) + " " + to_string(getNoteCount()));
    }
  }
  
  //logII(LL_CRIT, getNoteCount());
  //logII(LL_CRIT, lineVerts.size());
}
