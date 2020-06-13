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

note::note() : tempo(0), col(0), duration(0), x(0), y(0), render(false) {}

void note::init(double tempo, double time, double pitch, double duration, unsigned char color) {
  this->tempo = tempo;
  this->x = time;
  this->y = pitch;
  this->duration = duration;
  this->col = color;
}

void note::shift(int shift_x) {
  this->x  += shift_x* 12;
}

void note::updateTempo(int tempo) {
  this->x -= round(static_cast<double>(tempo)/7.5);
}

void note::scaleToWindow(int height, int range) {
  height = round(static_cast<double>(height)/range);
}

mfile::mfile() : noteCount(0), noteMin(0), noteMax(0), notes(nullptr) {}

mfile::~mfile() {
  delete []notes;
}

note* mfile::getNotes() {
  return notes;
}

int mfile::getNoteCount() {
  return noteCount;
}

void mfile::shift(int shift_x) {
  for (int i = 0; i < noteCount; i++) {
    notes->shift(shift_x);
  }
}

void mfile::updateTempo(int tempo_global) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].updateTempo(tempo_global);
  }
}

void mfile::scaleToWindow(int height) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].scaleToWindow(height, max(MIN_NOTE_HEIGHT, noteMax - noteMin));
  }
}

int::mfile::getNoteRange() {
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
       notes[idx].col = i;
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
