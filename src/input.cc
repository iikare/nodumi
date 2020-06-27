#include <vector>
#include <iostream>
#include "../dpd/rtmidi/RtMidi.h"
#include "input.h"
#include "note.h"

using std::vector;
using std::cerr;
using std::endl;

MidiInput::MidiInput() : msgQueue(0), numPort(0), curPort(0), noteCount(0), timestamp(0) {
  midiIn = new RtMidiIn();

  // eventually will be converted to dynamic vector
  noteStream = new mfile(10000);
  if (midiIn == nullptr) {
    cerr << "warn: could not initialize midi input" << endl;
  }
}

MidiInput::~MidiInput() {
  delete midiIn;
  delete noteStream;
}

void MidiInput::openPort(int port) {
  if (curPort != port) {
    curPort = port;
    numPort = midiIn->getPortCount();
    if (port >= numPort) {
      cerr << "warn: unable to open port number " << port << endl;
      return;
    }
    midiIn->openPort(port);
    midiIn->ignoreTypes(false, false, false);
  }
}

void MidiInput::updateQueue() {
  timestamp = midiIn->getMessage(&msgQueue);
  for (long unsigned int i = 0; i < msgQueue.size(); i++) {
    if ((int)msgQueue[i] != 248 && (int)msgQueue[i] != 254){ 
      cerr << "byte " << i << " is " << (int)msgQueue[i] << ", ";
    }

  }
  if (msgQueue.size() > 0) {
    if ((int)msgQueue[0] != 248 && (int)msgQueue[0] != 254){ 
      cerr << "timestamp: " << timestamp << endl;
    }
  }
}

void MidiInput::convertEvents() {
  for (long unsigned int i = 0; i < msgQueue.size(); i++) { 
    if (msgQueue[i] == 0b11111000) { // 248: clock signal
      //cerr << "shift by " << timestamp*100 << endl;
      noteStream->shiftX(-timestamp*100);
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;
        tmpNote.track = 1; // by default
        tmpNote.tempo = 120; 
        tmpNote.x = 0;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = 500;
        noteStream->notes[noteCount] = tmpNote;
        noteCount++;
        i += 2;
        cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
        noteStream->noteCount = noteCount;
      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[i + 1]));
        noteStream->notes[idx].duration = -noteStream->notes[idx].x;
        note tmpNote = noteStream->notes[idx];
        cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
      }
    }
  }
}

int MidiInput::findNoteIndex(int key) {
  for (int i = noteCount - 1; i >= 0; i--) {
    if (noteStream->notes[i].y == key) {
      return i;
    }
  }
  return 0;
}

void MidiInput::update() {
  updateQueue();
  convertEvents();
}