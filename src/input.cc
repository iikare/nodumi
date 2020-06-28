#include <vector>
#include <string>
#include <iostream>
#include "../dpd/rtmidi/RtMidi.h"
#include "input.h"
#include "note.h"

using std::to_string;
using std::vector;
using std::string;
using std::cerr;
using std::endl;

MidiInput::MidiInput() : noteStream(nullptr), midiIn(nullptr), msgQueue(0), numPort(0), curPort(0), noteCount(0), numOn(0), timestamp(0) {
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
    midiIn->closePort();
    curPort = port;
    numPort = midiIn->getPortCount();
    if (port >= numPort) {
      cerr << "warn: unable to open port number " << port << endl;
      return;
    }
    midiIn->openPort(port);
    midiIn->ignoreTypes(false, false, false);
    cerr << "info: opened port " << port << endl;
  }
  else {
    cerr << "info: attempted to open an already open port" << endl;
  }
}

vector<string> MidiInput::getPorts() {
  vector<string> ports;
  numPort = midiIn->getPortCount();
  for (int i = 0; i < numPort; i++) {
    ports.push_back(midiIn->getPortName(i));

    int sp = 0;
    for (unsigned int j = 0; j < ports[i].length(); j++) {
      // break after second space/colon
      if (ports[i][j] == ' ' || ports[i][j] == ':') {
        sp++;
      }
      if (sp == 2) {
        ports[i] = ports[i].substr(0, j);
        break;
      }
    }
    ports[i].insert(0, to_string(i) + ": ");
  }
  
  return ports;
}

bool MidiInput::updateQueue() {
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
    return true;
  }
  return false;
}

void MidiInput::convertEvents() {
  for (long unsigned int i = 0; i < msgQueue.size(); i++) { 
    if (msgQueue[i] == 0b11111000) { // 248: clock signal
      //cerr << "shift by " << timestamp*100 << endl;
      noteStream->shiftX(-timestamp * 100 * noteStream->getTimeScale());
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;
        tmpNote.track = 1; // by default
        tmpNote.tempo = 120; 
        tmpNote.x = 0;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        tmpNote.isOn = true;
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = 500;
        
        noteStream->notes[noteCount] = tmpNote;
        noteCount++;
        numOn++;
        i += 2;
        
        cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
        
        noteStream->noteCount = noteCount;
      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[i + 1]));
        noteStream->notes[idx].isOn = false;
        note tmpNote = noteStream->notes[idx];

        numOn--;

        cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
      }
    }
  }
}

void MidiInput::updatePosition() {
  int i = 0;
  for (int j = noteCount - 1; j >= 0; j--) {
    if (i >= numOn) {
      // all on notes shifted
      break;
    }
    if (noteStream->notes[j].isOn) {
      noteStream->notes[j].duration = -noteStream->notes[j].x;
      i++;
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
  while (updateQueue()) {
    convertEvents();
    updatePosition();
  }
}
