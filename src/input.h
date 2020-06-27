#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include <string>
#include "../dpd/rtmidi/RtMidi.h"
#include "note.h"

using std::vector;
using std::string;

class MidiInput {
  public:
    MidiInput();
    ~MidiInput();

    void openPort(int port);
    void update();
    
    note* getNotes() { return noteStream->getNotes(); }
    int getNoteCount() { return noteCount; }
    vector<string> getPorts();

  private:
    void convertEvents();
    void updateQueue();
    int findNoteIndex(int key);
    
    RtMidiIn* midiIn;
    mfile* noteStream;
    vector<unsigned char> msgQueue;
    int numPort;
    int curPort;
    int noteCount;
    double timestamp;

};

#endif
