#include "input.h"
#include "data.h"

midiInput::midiInput() : midiIn(nullptr), msgQueue(0), numPort(0), noteCount(0), numOn(0), timestamp(0) {
  midiIn = new RtMidiIn();
  logII(LL_CRIT, "owo");
  if (midiIn == nullptr) {
    logII(LL_WARN, "unable to initialize midi input");
  }
}

midiInput::~midiInput() {
  delete midiIn;
}

void midiInput::openPort(int port) {
  if (ctr.getLiveState()) {
    midiIn->closePort();

    numPort = midiIn->getPortCount();
    if (port >= numPort) {
      log3(LL_WARN, "unable to open port number", port);
      return;
    }

    midiIn->openPort(port);
    midiIn->ignoreTypes(false, false, false);
    
    log3(LL_INFO, "opened port ", port);
  }
  else {
    logII(LL_WARN, "cannot open port in normal mode");
  }
}

vector<string> midiInput::getPorts() {
  vector<string> ports;
  numPort = midiIn->getPortCount();
  for (int i = 0; i < numPort; i++) {
    ports.push_back(midiIn->getPortName(i));

    int sp = 0;
    for (unsigned int j = 0; j < ports[i].length(); j++) {
      // break after second space/colon
      if (ports[i][j] == ' ' || ports[i][j] == ':') {
        sp++;
      }
      if (sp == 2) {
        ports[i] = ports[i].substr(0, j);
        break;
      }
    }
    ports[i].insert(0, to_string(i) + ": ");
  }
  
  return ports;
}

bool midiInput::updateQueue() {
  timestamp = midiIn->getMessage(&msgQueue);
  for (long unsigned int i = 0; i < msgQueue.size(); i++) {
    if ((int)msgQueue[i] != 248 && (int)msgQueue[i] != 254){ 
      //cerr << "byte " << i << " is " << (int)msgQueue[i] << ", ";
    }

  }
  if (msgQueue.size() > 0) {
    if ((int)msgQueue[0] != 248 && (int)msgQueue[0] != 254){ 
      //cerr << "timestamp: " << timestamp << endl;
    }
    return true;
  }
  return false;
}

void midiInput::convertEvents() {
  //-timestamp * 100 * noteStream->getTimeScale()
  for (long unsigned int i = 0; i < msgQueue.size(); i++) { 
    if (msgQueue[i] == 0b11111000) { // 248: clock signal
      //cerr << "shift by " << timestamp*100 << endl;
      ctr.livePlayOffset += timestamp * 100;
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;
        tmpNote.track = 1; // by default
        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        tmpNote.isOn = true;
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = -1;
        
        noteStream.notes.push_back(tmpNote);
        noteCount++;
        numOn++;
        i += 2;
        
        //cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
        
        noteStream.noteCount = noteCount;
      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[i + 1]));
        noteStream.notes[idx].isOn = false;
        //note tmpNote = noteStream.notes[idx];

        numOn--;

        //cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
      }
    }
  }
}

void midiInput::updatePosition() {
  int i = 0;
  for (int j = noteCount - 1; j >= 0; j--) {
    if (i >= numOn) {
      // all on notes shifted
      break;
    }
    if (noteStream.notes[j].isOn) {
      noteStream.notes[j].duration = ctr.livePlayOffset - noteStream.notes[j].x;
      i++;
    }
  }
}

int midiInput::findNoteIndex(int key) {
  for (int i = noteCount - 1; i >= 0; i--) {
    if (noteStream.notes[i].y == key) {
      return i;
    }
  }
  return 0;
}

void midiInput::update() {
  if (midiIn->isPortOpen()) {
    while (updateQueue()) {
      convertEvents();
      updatePosition();
    }
  }
  else {
    // shift even when midi input is disconnected
    ctr.livePlayOffset += GetFrameTime();
  }
}
