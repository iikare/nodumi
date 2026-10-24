#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "../dpd/rtmidi/RtMidi.h"
#include "log.h"
#include "midi.h"
#include "note.h"

using std::make_unique;
using std::string;
using std::unique_ptr;
using std::vector;

class midiOutput {
 public:
  midiOutput();

  // named so the input side can recognise this app's own output and never read it back
  static constexpr const char* client_name = "nodumi output";

  bool isPortOpen() { return midiOut->isPortOpen(); }
  void openPort(int port);
  void update();
  void sendMessage(vector<unsigned char>* msgQueue);

  vector<string> getPorts();
  const string& get_port_name() const { return port_name; }

 private:
  unique_ptr<RtMidiOut> midiOut;
  string port_name;
  vector<unsigned char> msgQueue;
  int numPort;
  int curPort;
};
