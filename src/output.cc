#include "output.h"

#include <memory>

#include "misc.h"

midiOutput::midiOutput() : midiOut(nullptr), msgQueue(0), numPort(0), curPort(-1) {
  midiOut = make_unique<RtMidiOut>(RtMidi::UNSPECIFIED, client_name);
  if (midiOut == nullptr) {
    logW(LL_WARN, "unable to initialize MIDI output");
  }
}

void midiOutput::openPort(int port) {
  lock_guard<mutex> lock(crit);
  numPort = midiOut->getPortCount();
  if (port < 0 || port >= numPort) {
    logW(LL_WARN, "unable to open port number", port);
    return;
  }

  midiOut->closePort();

  port_name = midiOut->getPortName(port);
  midiOut->openPort(port, port_name);

  curPort = port;

  logW(LL_INFO, "[OUT] opened port", port);
}

vector<string> midiOutput::getPorts() {
  lock_guard<mutex> lock(crit);
  vector<string> ports;
  numPort = midiOut->getPortCount();
  ports.resize(numPort);
  for (int i = 0; i < numPort; i++) {
    ports[i] = midiOut->getPortName(i);
  }

  return formatPortName(ports);
}

void midiOutput::sendMessage(vector<unsigned char>* msgQueue) {
  lock_guard<mutex> lock(crit);
  if (curPort != -1 && midiOut->isPortOpen()) {
    midiOut->sendMessage(msgQueue);
  }
}
