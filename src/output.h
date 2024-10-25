#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "../dpd/rtmidi/RtMidi.h"
#include "log.h"
#include "midi.h"
#include "note.h"

using std::string;
using std::unique_ptr;
using std::vector;

class midiOutput {
 public:
  midiOutput();

  bool isPortOpen() { return midiOut->isPortOpen(); }
  void openPort(int port);
  void update();
  void sendMessage(vector<unsigned char>* msgQueue);

  vector<string> getPorts();

 private:
  unique_ptr<RtMidiOut> midiOut;
  vector<unsigned char> msgQueue;
  int numPort;
  int curPort;
};
