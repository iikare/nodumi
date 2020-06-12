#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include "note.h"

using namespace smf;
using std::cerr;
using std::endl;
using std::string;

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

void note::update(int tempo) {
  this->x -= round(static_cast<double>(tempo)/7.5);
}

mfile::mfile() : noteCount(0), notes(nullptr) {}

mfile::~mfile() {
  delete []notes;
}

note* mfile::getAllNotes() {
  return notes;
}

int mfile::getNumNotes() {
  return noteCount;
}

void mfile::shift(int shift_x) {
  for (int i = 0; i < noteCount; i++) {
    notes->shift(shift_x);
  }
}

void mfile::update(int tempo_global) {
  for (int i = 0; i < noteCount; i++) {
    notes[i].update(tempo_global);
  }
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
      cerr << "uwu" << endl; 
      if (midifile[i][j].isNoteOn()) {
  cerr << noteCount << endl;
        noteCount++;
      }
    }
  }
  cerr << "this is a new owo" << endl;
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
        //std::swap(notes[i], notes[j]);
      }
    }
  }

  idx = 0;
  
  //midifile.joinTracks();
  //midifile.sortTracks();

  for (int i = 0; i < midifile.getEventCount(0); i++) {
    if (midifile[0][i].isTempo()) {
      bpm = midifile[0][i].getTempoBPM();
    }
    if (midifile[0][i].isNoteOn()) {
      notes[idx].tempo = bpm;
      idx++;
    }
  }
}
