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

using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

class midiInput {
 public:
  midiInput();

  void initClassifier();

  void openPort(int port, bool pauseEvent = false);
  void resetInput(bool keepPort = false);
  void update();
  void pauseInput();
  void resumeInput();
  void terminate();
  void reset_lstm_count();
  bool lstm_enabled() { return classifier->enabled(); }

  int findKeySig();
  string findKeySigLabel();

  int getNoteCount() { return noteCount; }
  vector<string> getPorts();

  static constexpr const char* client_name = "nodumi input";

  midi noteStream;

 private:
  void convertEvents();
  void updatePosition();
  bool updateQueue();
  bool isUntimedQueue();
  int findNoteIndex(int key);
  int findPartition(const note& n);

  void resetStream();

  unique_ptr<RtMidiIn> midiIn;
  vector<unsigned char> msgQueue;
  int numPort;
  int curPort;
  int noteCount;
  int numOn;
  double timestamp;
  bool frame_advanced = false;
  bool notes_changed = false;
  string port_name;
  vector<int> port_map;  // menu position to midi port, leaving out this app's own output

  vector<pair<int, keySig>> match;
  int ks_type;

  int notes_since_lstm_enable = 0;

  unique_ptr<inputClassifier> classifier;
  bool classifier_warmup = false;
};
