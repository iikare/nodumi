#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include "../dpd/rtmidi/RtMidi.h"
#include "note.h"
#include "midi.h"
#include "log.h"

using std::vector;
using std::string;
using std::unique_ptr;

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
