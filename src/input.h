#ifndef INPUT_H
#define INPUT_H

#include <vector>
#include "../dpd/rtmidi/RtMidi.h"

using std::vector;

class MidiInput {
  public:
    MidiInput();
    ~MidiInput();
    void openPort(int port);
    void updateQueue();

  private:
    RtMidiIn* midiIn;
    vector<unsigned char> msgQueue;
    int numByte;
    int numPort;
    double timestamp;

};

#endif
