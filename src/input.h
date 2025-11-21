#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../dpd/rtmidi/RtMidi.h"
#include "classifier.h"
#include "log.h"
#include "midi.h"
#include "note.h"
#include "track_split.h"

using std::string;
using std::unique_ptr;
using std::vector;

class midiInput {
 public:
  midiInput();

  void initClassifier();

  void openPort(int port, bool pauseEvent = false);
  void resetInput();
  void update();
  void pauseInput();
  void resumeInput();
  void terminate();

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

  unique_ptr<inputClassifier> classifier;
  bool classifier_warmup = false;
};
