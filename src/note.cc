#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "file.h"
#include "misc.h"
#include "note.h"

using namespace smf;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::max;
using std::min;
using std::swap;
using std::sort;
using std::pair;
using std::make_pair;

note::note() : track(0),  duration(0), x(0), y(0), velocity(0), time(0), isOn(false) {}

note::~note() {}

note::note(const note& nNote) {
  track = nNote.track;
  x = nNote.x;
  y = nNote.y;
  duration = nNote.duration;
  velocity = nNote.velocity;
  time = nNote.time;
  isOn = nNote.isOn;
}

void note::operator = (const note& nNote) {
  track = nNote.track;
  x = nNote.x;
  y = nNote.y;
  duration = nNote.duration;
  velocity = nNote.velocity;
  time = nNote.time;
  isOn = nNote.isOn;
}

void note::shiftX(double shiftX) {
  x += shiftX;
}

void note::shiftTime(double timeInc) {
  x -= timeInc/TIME_MODIFIER;
}

void note::scaleTime(double timeScale) {
  x *= timeScale;
  duration *= timeScale;
}

mfile::mfile() : trackCount(0), noteCount(0), noteMin(0), noteMax(0), timeScale(1), lastTick(0), lastTime(0), tempoMap(0), notes(nullptr) {}

mfile::mfile(int bufSize) : trackCount(0), noteCount(0), noteMin(0), noteMax(0), timeScale(1), lastTick(0), lastTime(0), tempoMap(0), notes(nullptr) {
  notes = new note[bufSize];
}

mfile::~mfile() {
  delete[] notes;
  notes = nullptr;
}

note* mfile::getNotes() {
  return notes;
}

int mfile::getNoteCount() {
  return noteCount;
}

void mfile::shiftX(double shiftX) {
  //cerr << "timeScale is " << timeScale << " from mfile::shiftX" << endl;
  //cerr << "shiftX is " << shiftX << " from mfile::shiftX" << endl;
  for (int i = 0; i < noteCount; i++) {
    notes[i].shiftX(shiftX);
  }
}

void mfile::shiftTime(double timeInc) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].shiftTime(timeInc);
  }
}

void mfile::scaleTime(double timeScale) {
  this->timeScale *= timeScale;
  //cerr << "this timeScale is " << timeScale << endl;
  //cerr << "vs.  " << this->timeScale << endl;
  for (int i = 0; i < noteCount; i++) {
    notes[i].scaleTime(timeScale);
  }
}

int mfile::getNoteRange() {
  int result = noteMax - noteMin;
  if (result <= 0) {
    cerr << "error: MIDI file with note range 0" << endl;
    exit(1);
  }
  return result;
}

double mfile::getTimeScale() {
  return timeScale;
}

double mfile::getLastTick() {
  return notes[noteCount - 1].x * timeScale;
}

double mfile::getLastTime() {
  return lastTime;
}

int mfile::getTrackCount() {
  return trackCount;
}

note mfile::findCurrentNote() {
  for (int i = 0; i < noteCount; i++) {
    if (notes[0].x < 0 && notes[2].x > 0) {
      return notes[1];
    }
    else if (notes[i].x >= 0) {
      return notes[i-1];
    }
    else if (i + 1 == noteCount) {
      return notes[i];
    }
  }
  cerr << "warn: findCurrentNote() failed with idx 0" << endl;
  return notes[0];
}

int mfile::findChordY(int idx) {
  if (idx < 0 || idx > noteCount) {
    cerr << "warn: invalid call to findChordY() with index " << idx << endl;
  }

  // break on end of track note
  if (idx + 1 < noteCount && notes[idx].track != notes[idx + 1].track) {
    return notes[idx].y;
  }
  
  int offset = 1;
  int y = notes[idx].y;
  int matchCount = 1;
  double epsilon = 0.0001;

  while (offset < 10) {
    // handle negative case
    if (idx - offset > 0) {
      if (notes[idx].track == notes[idx - offset].track) {
        if (fabs(notes[idx].x - notes[idx - offset].x) < epsilon) {
          y += notes[idx - offset].y;
          matchCount++;
        }
      }
    }
    // handle positive case
    if (idx + offset < noteCount) {
      if (notes[idx].track == notes[idx + offset].track) {
        if (fabs(notes[idx].x - notes[idx + offset].x) < epsilon) {
          y += notes[idx + offset].y;
          matchCount++;
        }
      }
    }
    offset++;
  }

  if (matchCount > 1) {
    //cerr << "note " << idx << " is in a chord with " << matchCount << " notes" << endl;
    //cerr << "^ has y " << round(y / matchCount) << endl;
  }

  return round(y / matchCount);
}

int mfile::findCurrentTempo() {
  double firstX = -notes[0].x / timeScale;
  //cerr<< firstX << endl;
  for (unsigned int i = 0; i < tempoMap.size(); i++) {
    if (i + 1 == tempoMap.size() || (firstX >= tempoMap[i].first && firstX < tempoMap[i + 1].first)) {
     return tempoMap[i].second;
    }
  }
  // default tempo for no tempo map
  return 120; 
}

void mfile::load(string file) {
  if (notes != nullptr) {
    cerr << "info: resetting event structure" << endl;
   
    //delete[] notes; 
    notes = nullptr;
    
    tempoMap.clear();

    noteCount = 0;
    noteMin = 0;
    noteMax = 0;
    timeScale = 1.25;
  }

  cerr << "info: loading MIDI - " << file << endl;

  MidiFile midifile;
  
  if (!midifile.read(file.c_str())) {
    cerr << "error: unable to open MIDI" << endl;
    exit(1);
  }

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
    cerr << "error: zero length file" << endl;
    exit(1);
  }

  sort(trackInfo.begin(), trackInfo.end());

  notes = new note[noteCount];

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

        if (idx != 0 && notes[idx].x < notes[idx - 1].x) {
          cerr << "warn: note with index " << idx << " is misaligned (" << notes[idx - 1].x << " -> " << notes[idx].x << ")" << endl;
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

  //for (unsigned int i = 0; i < tempoMap.size(); i++) {
    //cerr << "tick: "<< tempoMap[i].first << " bpm: " << tempoMap[i].second << endl;
  //}

  // determine scaling factor
  // has been removed to improve loading times - function should be independent of loading
  /*
  for (int i = 0; i < static_cast<int>(sizeof(notes)); i++) {
    if(notes[i].y < MIN_NOTE_IDX || notes[i].y > MAX_NOTE_IDX) {
      cerr << "warn: note " << i << " is out of bounds with index " << notes[i].y << endl;
    }

    noteMin = min(noteMin, notes[i].y);
    noteMax = max(noteMax, notes[i].y);
  }
  */

  // get last note tick and time  
  lastTick = notes[noteCount-1].x + notes[noteCount-1].duration;
  lastTime = midifile[0].last().seconds;

  // scale for visibility
  scaleTime(static_cast<double>(1)/8);

  // this way, the starting tick is by definition 0 , and the ending tick is the old first tick
}
