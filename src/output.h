#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../dpd/rtmidi/RtMidi.h"
#include "log.h"
#include "midi.h"
#include "note.h"

using std::lock_guard;
using std::make_unique;
using std::mutex;
using std::string;
using std::unique_ptr;
using std::vector;

class midiOutput {
 public:
  midiOutput();

  // named so the input side can recognise this app's own output and never read it back
  static constexpr const char* client_name = "nodumi output";

  bool isPortOpen() {
    lock_guard<mutex> lock(crit);
    return midiOut->isPortOpen();
  }
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

  // the output thread sends while the main thread switches ports or echoes live input
  mutex crit;
};
