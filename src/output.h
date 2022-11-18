#pragma once

#include <vector>
#include <string>
#include "../dpd/rtmidi/RtMidi.h"
#include "note.h"
#include "midi.h"
#include "log.h"

using std::vector;
using std::string;

class midiOutput {
  public:
    midiOutput();
    ~midiOutput();


    bool isPortOpen() { return midiOut->isPortOpen(); }
    void openPort(int port);
    void update();
    void sendMessage(vector<unsigned char>* msgQueue);
    
    vector<string> getPorts();

  private:
    
    RtMidiOut* midiOut;
    vector<unsigned char> msgQueue;
    int numPort;
    int curPort;
};
