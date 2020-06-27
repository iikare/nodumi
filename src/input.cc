#include <vector>
#include <iostream>
#include "../dpd/rtmidi/RtMidi.h"
#include "input.h"

using std::vector;
using std::cerr;
using std::endl;

MidiInput::MidiInput() : msgQueue(0), numByte(0), numPort(0), timestamp(0) {
  midiIn = new RtMidiIn();
  if (midiIn == nullptr) {
    cerr << "warn: could not initialize midi input" << endl;
  }
}

MidiInput::~MidiInput() {
  delete midiIn;
}

void MidiInput::openPort(int port) {
  numPort = midiIn->getPortCount();
  if (port >= numPort) {
    cerr << "warn: unable to open port number " << port << endl;
    return;
  }
  midiIn->openPort(port);
  midiIn->ignoreTypes(false, false, false);
}

void MidiInput::updateQueue() {
  timestamp = midiIn->getMessage(&msgQueue);
  numByte = msgQueue.size();
  for (int i = 0; i < numByte; i++) {
    cerr << "byte " << i << " is " << (int)msgQueue[i] << ", ";
  }
  if (numByte > 0) {
    cerr << "timestamp: " << timestamp << endl;
  }
}
