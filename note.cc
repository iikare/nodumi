#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

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

note::note() : track(0), tempo(0),  duration(0), x(0), y(0) {}

void note::init(int track, double tempo, double x, int y, double duration) {
  this->track = track;
  this->tempo = tempo;
  this->x = x;
  this->y = y;
  this->duration = duration;
}

void note::shiftX(int shiftX) {
  x += shiftX;
}

void note::shiftTime(int timeInc) {
  x -= floor(static_cast<double>(timeInc)/8);
}

mfile::mfile() : noteCount(0), noteMin(0), noteMax(0), notes(nullptr) {}

mfile::~mfile() {
  delete[] notes;
}

note* mfile::getNotes() {
  return notes;
}

int mfile::getNoteCount() {
  return noteCount;
}

void mfile::shiftX(int shiftX) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].shiftX(shiftX);
  }
}

void mfile::shiftTime(int timeInc) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].shiftTime(timeInc);
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

void mfile::load(string file) {
  if (notes != nullptr) {
    cerr << "info: resetting event structure" << endl;
    delete []notes;
    noteCount = 0;
  }

  cerr << "info: loading MIDI - " << file << endl;

  MidiFile midifile;
  midifile.read(file.c_str());

  midifile.linkNotePairs();
 
  int tracks = midifile.getTrackCount();
  int cTrackCount = 0;

  for (int i = 0; i < tracks; i++) {
    cTrackCount = midifile.getEventCount(i);
    for (int j = 0; j < cTrackCount; j++) {
      if (midifile[i][j].isNoteOn()) {
        noteCount++;
      }
    }
  }
  notes = new note[noteCount];

  int bpm = 0;
  int idx = 0;

  for (int i = 0; i < tracks; i++) {
    for (int j = 0; j < midifile.getEventCount(i); j++) {
      if (midifile[i][j].isNoteOn()) {
       notes[idx].track = i;
       notes[idx].duration = midifile[i][j].getTickDuration();
       notes[idx].x  = midifile[i][j].tick;
       notes[idx].y = midifile[i][j].getKeyNumber();

       idx++;
      }
    }
  }
  
  for (int i = 0; i < noteCount; i++) {
    for (int j = 0; j < noteCount; j++) {
      if(notes[j].x < notes[i].x) {
        std::swap(notes[i], notes[j]);
      }
    }
  }

  idx = 0;
  
  midifile.joinTracks();
  midifile.sortTracks();

  for (int i = 0; i < midifile.getEventCount(0); i++) {
    if (midifile[0][i].isTempo()) {
      bpm = midifile[0][i].getTempoBPM();
    }
    if (midifile[0][i].isNoteOn()) {
      notes[idx].tempo = bpm;
      idx++;
    }
  } 

  // determine scaling factor
  for (int i = 0; i < static_cast<int>(sizeof(notes)); i++) {
    if(notes[i].y < MIN_NOTE_IDX || notes[i].y > MAX_NOTE_IDX) {
      cerr << "warn: note " << i << " is out of bounds with index " << notes[i].y << endl;
    }

    noteMin = min(noteMin, notes[i].y);
    noteMax = max(noteMax, notes[i].y);
  }
}
