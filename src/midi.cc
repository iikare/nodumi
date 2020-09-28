#include <algorithm>
#include "midi.h"

using std::max;

void midi::load(string file) {
  MidiFile midifile;
  if (!midifile.read(file.c_str())) {
    logII(LL_WARN, "unable to open MIDI");
    return;
  }
  notes.clear();
  tempoMap.clear();
  noteCount = 0;
  trackCount = 0;

  midifile.linkNotePairs();
 
  midifile.doTimeAnalysis();
  
  trackCount = midifile.getTrackCount();

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
        notes[idx].track = i;
        notes[idx].duration = midifile[i][j].getDurationInSeconds() * 500;
        notes[idx].x  = midifile[i][j].seconds * 500;
        notes[idx].y = midifile[i][j].getKeyNumber();
        notes[idx].velocity = midifile[i][j][2];
        notes[idx].time = midifile[i][j].seconds;

        lastTick = max(lastTick, notes[idx].x + notes[idx].duration);

        if (idx != 0 && notes[idx].x < notes[idx - 1].x) {
          logII(LL_WARN, "note with index " + to_string(idx) + " is misaligned (" +
                to_string(notes[idx - 1].x) + " -> " + to_string(notes[idx].x) + ")");
        } 
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
    if (midifile[0][i].isNoteOn()) {
      lastIdx = idx;
      idx++;
    }
  }

  lastTick = notes[getNoteCount() - 1].x + notes[getNoteCount() - 1].duration;
}
