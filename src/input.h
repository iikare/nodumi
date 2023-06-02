#pragma once

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

class midiInput {
  public:
    midiInput();

    void openPort(int port, bool pauseEvent = false);
    void resetInput();
    void update();
    void pauseInput();
    void resumeInput();
    
    int getNoteCount() { return noteCount; }
    vector<string> getPorts();

    midi noteStream;

  private:
    void convertEvents();
    void updatePosition();
    bool updateQueue();
    bool isUntimedQueue();
    int findNoteIndex(int key);
    int findPartition(const note& n);
    
    unique_ptr<RtMidiIn> midiIn;
    vector<unsigned char> msgQueue;
    int numPort;
    int curPort;
    int noteCount;
    int numOn;
    double timestamp;

};
