#include<algorithm>
#include<bitset>
#include "input.h"
#include "data.h"
#include "controller.h"
#include "define.h"
#include "log.h"

midiInput::midiInput() : midiIn(nullptr), msgQueue(0), numPort(0), curPort(-1), noteCount(0), numOn(0), timestamp(0) {
  midiIn = unique_ptr<RtMidiIn>(new RtMidiIn());
  if (midiIn == nullptr) {
    logW(LL_WARN, "unable to initialize midi input");
  }
  for (auto& t : noteStream.getTracks()){
    t.setNoteVector(&noteStream.notes);
  }
}

void midiInput::openPort(int port, bool pauseEvent) {
  if (!pauseEvent) {
    resetInput();
  }

  numPort = midiIn->getPortCount();
  if (port < 0 || port >= numPort) {
    logW(LL_WARN, "unable to open port number", port);
    return;
  }

  midiIn->openPort(port, midiIn->getPortName(port));
  midiIn->ignoreTypes(false, false, false);

  curPort = port;
  timestamp = midiIn->getMessage(&msgQueue);

  if (!pauseEvent) {
    logW(LL_INFO, "[IN] opened port", port);
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

  for (unsigned int i = 0; i < noteStream.getTracks().size(); i++) {
    noteStream.getTracks()[i].reset();
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
  int tempTS = midiIn->getMessage(&msgQueue);
  if (isUntimedQueue()) {
    timestamp += GetFrameTime();
    ctr.livePlayOffset += GetFrameTime() * UNK_CST;
  }
  else {
    timestamp = tempTS;
  }
    

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

bool midiInput::isUntimedQueue() {
  bool noShift = true;
  for (long unsigned int i = 0; i < msgQueue.size(); i++) { 
    if (msgQueue[i] == 0b11111000) {
      noShift = false;
      break;
    }
  }
  return noShift;
}

void midiInput::convertEvents() {
  //-timestamp * 100 * noteStream->getTimeScale()
  //logQ(" ");

  //logQ(ctr.livePlayOffset, timestamp);
  for (long unsigned int i = 0; i < msgQueue.size(); i++) { 
    //logQ(bitset<8>(msgQueue[i]));
   
    //logQ(ctr.livePlayOffset);

    if (msgQueue[i] == 0b11111000) { // 248: clock signal
      //cerr << "shift by " << timestamp*100 << endl;
      ctr.livePlayOffset += fmax(0, timestamp) * UNK_CST;
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;

        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        tmpNote.isOn = true;
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = -1;

        //tmpNote.track = ctr.option.get(OPTION::TRACK_DIVISION) ? findPartition(tmpNote) : 1; // by default
       
        //logQ("count, x, track, trackct:", noteCount, tmpNote.x, tmpNote.track, 
             //noteStream.getTracks()[tmpNote.track].getNoteCount());

        // partition finder requires the current note to be present
        noteStream.notes.push_back(tmpNote);
        
        numOn++;
        tmpNote.track = ctr.option.get(OPTION::TRACK_DIVISION) ? findPartition(tmpNote) : 1; // by default
                                                                                                        
        // update track after determination
        noteStream.notes[noteCount].track = tmpNote.track;

        noteStream.getTracks()[tmpNote.track].insert(noteCount);
        
        // update last index only AFTER track splitter
        noteCount++;
        


       


        i += 2;
        
        //cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
        
        noteStream.setNoteCount(noteCount);

      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[i + 1]));
        noteStream.notes[idx].isOn = false;
        //note tmpNote = noteStream.notes[idx];

        numOn--;

        //cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " << tmpNote.y << ", " << tmpNote.velocity << endl;
      }

        for (auto& t : noteStream.getTracks()){
          t.buildChordMap();
        }
        noteStream.buildLineMap();
        //logQ("line size:", noteStream.lines.size());
    }
  }
}

int midiInput::findPartition(const note& n) {
  //logW(LL_WARN, "new note @", n.y);
  return findTrack(n, noteStream, true, numOn);
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
  if (ctr.getLiveState()) {
    if (midiIn->isPortOpen()) {
      while (updateQueue()) {
        convertEvents();
        updatePosition();
      }
      // TODO: consider optimization
      //ctr.input.noteStream.buildLineMap();
    }
    else {
      // shift even when midi input is disconnected
      ctr.livePlayOffset += GetFrameTime() * UNK_CST;
    }
  }
  else {
    // empty midi queue
    while (true) {
      midiIn->getMessage(&msgQueue);
      if (!msgQueue.size()) {
        break;
      }
    }
  }
}
