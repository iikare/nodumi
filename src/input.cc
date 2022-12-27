#include<algorithm>
#include<bitset>
#include "input.h"
#include "data.h"
#include "controller.h"
#include "define.h"

using std::min;
using std::max;
using std::sort;
using std::bitset;

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

    midiIn->openPort(port, midiIn->getPortName(port));
    midiIn->ignoreTypes(false, false, false);

    curPort = port;
    timestamp = midiIn->getMessage(&msgQueue);

    if (!pauseEvent) {
      logW(LL_INFO, "[IN] opened port", port);
    }
  }
  else {
    logW(LL_WARN, "cannot open input port in normal mode");
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
    ctr.livePlayOffset += GetFrameTime() * 500;
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
    if (msgQueue[i] == 0b11111000) { // 248: clock signal
      //cerr << "shift by " << timestamp*100 << endl;
      ctr.livePlayOffset += fmax(0, timestamp) * 500;
    }
    else if (msgQueue[i] == 0b10010000) { // 144: note on/off
      if (msgQueue[i + 2] != 0) { // if note on
        note tmpNote;


        


        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[i + 1]);
        tmpNote.velocity = static_cast<int>(msgQueue[i + 2]);
        tmpNote.isOn = true;
        

        tmpNote.track = ctr.option.get(optionType::OPTION_TRACK_DIVISION) ? findPartition(tmpNote.y) : 1; // by default
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = -1;
       
        //logQ("count, x, track, trackct:", noteCount, tmpNote.x, tmpNote.track, 
             //noteStream.getTracks()[tmpNote.track].getNoteCount());

        noteStream.notes.push_back(tmpNote);
        noteStream.getTracks()[tmpNote.track].insert(noteCount, &noteStream.notes.at(noteCount)); 
        noteCount++;
        numOn++;
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
    }
  }
}

int midiInput::findPartition(int y) {

  // default assumption for FIRST note in a sequence
  if (noteCount == 0 || numOn == 0) {
    if (y >= 60) {
      return 1;
    }
    return 0;
  }
  // find latest old note
  int i = 0;
  double minX = std::numeric_limits<double>::max();
  int minIdx = 0;
  for (int j = noteCount - 1; j >= 0; --j) {
    if (i >= numOn) {
      // all on notes shifted
      break;
    }
    if (noteStream.notes[j].isOn) {
      if (noteStream.notes[j].x < minX) {
        minX = noteStream.notes[j].x;
        minIdx = j;
      }
      i++;
    }
  }

  //minIdx = max(minIdx - 5, 0);

  // find span range
  int minY = 127;
  int maxY = 0;

  vector<int> relevantY;
  for (int j = minIdx; j < noteCount; ++j) {

    if (noteStream.notes[j].isOn) {
      maxY = max(maxY, noteStream.notes[j].y);
      minY = min(minY, noteStream.notes[j].y);


      relevantY.push_back(noteStream.notes[j].y);
    }

  }



  logQ("range:", minY, maxY);
  logQ("consider Y", relevantY);

  constexpr int maxRange = 16; // tenth range

  if (maxY-minY <= maxRange) {
  
    // use earliest position
    if (noteStream.notes[minIdx].y >= 60) {
      return 1;
    }
    else { 
      return 0;
    }
  }

  else {//if (relevantY.size() == 1) {
    if (y >= 60) {
      return 1;
    }
    return 0;
  }
  //else {

    //sort(relevantY.begin(), relevantY.end());


    //int maxDist = -1;
    //int sepIdx = 0;
    //for (unsigned long idx = 1; idx < relevantY.size(); ++idx) {


      //if (maxDist < relevantY[idx] - relevantY[idx-1]) {
        //maxDist = relevantY[idx] - relevantY[idx-1];
        //sepIdx = idx;
      //}
    //}


    //if (y >= noteStream.notes[sepIdx].y) {
      //return 1;
    //}
    //return 0;
    




  //}


  return 0;
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
    // TODO: consider optimization
    //ctr.liveInput.noteStream.buildLineMap();
  }
  else {
    // shift even when midi input is disconnected
    ctr.livePlayOffset += GetFrameTime() * 500;
  }
}
