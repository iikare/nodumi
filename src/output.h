#pragma once

#include <vector>
#include <string>
#include "../dpd/rtmidi/RtMidi.h"
#include "note.h"
#include "midi.h"
#include "log.h"

using std::vector;
using std::string;
using std::to_string;
class midiOutput {
  public:
    midiOutput();
    ~midiOutput();


    void openPort(int port);
    void update();
    
    vector<string> getPorts();

  private:
    
    RtMidiOut* midiOut;
    vector<unsigned char> msgQueue;
    int numPort;
    int curPort;
};
