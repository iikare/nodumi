#include "input.h"
#include "data.h"
#include "controller.h"
#include "define.h"

midiInput::midiInput() : midiIn(nullptr), msgQueue(0), numPort(0), curPort(-1), noteCount(0), numOn(0), timestamp(0) {
  midiIn = new RtMidiIn();
  if (midiIn == nullptr) {
    logW(LL_WARN, "unable to initialize midi input");
  }
}

midiInput::~midiInput() {
  delete midiIn;
}

void midiInput::openPort(int port, bool pauseEvent) {
  if (ctr.getLiveState()) {
    if (!pauseEvent) {
      resetInput();
    }

    numPort = midiIn->getPortCount();
    if (port < 0 || port >= numPort) {
      logW(LL_WARN, "unable to open port number", port);
      return;
    }

    midiIn->openPort(port);
    midiIn->ignoreTypes(false, false, false);

    curPort = port;
    timestamp = midiIn->getMessage(&msgQueue);

    if (!pauseEvent) {
      logW(LL_INFO, "[IN] opened port", port);
    }
  }
  else {
    logW(LL_WARN, "cannot open port in normal mode");
  }
}

void midiInput::resetInput() {
  ctr.livePlayOffset = 0;
  midiIn->closePort();
  msgQueue.clear();
  timestamp = 0;
  noteCount = 0;
  numOn = 0;
  curPort = 0;
  
  // TODO: implement midi reset handler
  noteStream.notes.clear();
  noteStream.measureMap.clear();
  noteStream.measureTickMap.clear();

  for (unsigned int i = 0; i < noteStream.tracks.size(); i++) {
    noteStream.tracks[i].reset();
  }
}

void midiInput::pauseInput() {
  midiIn->closePort();
}

void midiInput::resumeInput() {
  openPort(curPort, true);
}

vector<string> midiInput::getPorts() {
  vector<string> ports;
  numPort = midiIn->getPortCount();
  for (int i = 0; i < numPort; i++) {
    ports.push_back(midiIn->getPortName(i));
  }
  
  return formatPortName(ports);
}

bool midiInput::updateQueue() {
  timestamp = midiIn->getMessage(&msgQueue);


  for (long unsigned int i = 0; i < msgQueue.size(); i++) {
    if ((int)msgQueue[i] != 248 && (int)msgQueue[i] != 254){ 
      //cerr << "byte " << i << " is " << (int)msgQueue[i] << ", ";
    }

  }
  if (msgQueue.size() > 0) {
    ctr.output.sendMessage(&msgQueue);
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
      ctr.livePlayOffset += fmax(0, timestamp) * 100;
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;
        tmpNote.track = 0; // by default
        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        tmpNote.isOn = true;
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = -1;
       
        noteStream.notes.push_back(tmpNote);
        noteStream.tracks[0].insert(noteCount, &noteStream.notes.at(noteCount)); 
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
