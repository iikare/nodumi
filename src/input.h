#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include "../dpd/rtmidi/RtMidi.h"
#include "note.h"

using std::vector;

class MidiInput {
  public:
    MidiInput();
    ~MidiInput();

    void openPort(int port);
    void update();
    
    note* getNotes() { return noteStream->getNotes(); }
    int getNoteCount() { return noteCount; }

    int findNoteIndex(int key);

  private:
    void convertEvents();
    void updateQueue();
    RtMidiIn* midiIn;
    mfile* noteStream;
    vector<unsigned char> msgQueue;
    int numPort;
    int curPort;
    int noteCount;
    double timestamp;

};

#endif
