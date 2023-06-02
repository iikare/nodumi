#include "output.h"
#include "misc.h"
#include <memory>


midiOutput::midiOutput() : midiOut(nullptr), msgQueue(0), numPort(0), curPort(-1) {
  midiOut = unique_ptr<RtMidiOut>{new RtMidiOut()};
  if (midiOut == nullptr) {
    logW(LL_WARN, "unable to initialize midi output");
  }
}

void midiOutput::openPort(int port) {
  numPort = midiOut->getPortCount();
  if (port < 0 || port >= numPort) {
    logW(LL_WARN, "unable to open port number", port);
    return;
  }

  midiOut->closePort();

  midiOut->openPort(port, midiOut->getPortName(port));

  curPort = port;

  logW(LL_INFO, "[OUT] opened port", port);
}

vector<string> midiOutput::getPorts() {
  vector<string> ports;
  numPort = midiOut->getPortCount();
  ports.resize(numPort);
  for (int i = 0; i < numPort; i++) {
    ports[i] = midiOut->getPortName(i);
  }
  
  return formatPortName(ports);
}

void midiOutput::sendMessage(vector<unsigned char>* msgQueue) {
  if (curPort != -1 && midiOut->isPortOpen()) {
    midiOut->sendMessage(msgQueue);
  }
}
