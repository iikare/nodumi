#include<algorithm>
#include<bitset>
#include "input.h"
#include "data.h"
#include "controller.h"
#include "define.h"
#include "log.h"

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
   
    //logQ(ctr.livePlayOffset);

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
        
        // if this is the note on event, duration is undefined
        tmpNote.duration = -1;

        //tmpNote.track = ctr.option.get(optionType::OPTION_TRACK_DIVISION) ? findPartition(tmpNote) : 1; // by default
       
        //logQ("count, x, track, trackct:", noteCount, tmpNote.x, tmpNote.track, 
             //noteStream.getTracks()[tmpNote.track].getNoteCount());

        // partition finder requires the current note to be present
        noteStream.notes.push_back(tmpNote);
        
        numOn++;
        tmpNote.track = ctr.option.get(optionType::OPTION_TRACK_DIVISION) ? findPartition(tmpNote) : 1; // by default
                                                                                                        
        // update track after determination
        noteStream.notes[noteCount].track = tmpNote.track;


        noteStream.getTracks()[tmpNote.track].insert(noteCount, &noteStream.notes.at(noteCount)); 
        
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
    }
  }
}

int midiInput::findPartition(const note& n) {
  //logW(LL_WARN, "new note @", n.y);
  constexpr int seqLimit = 2000;

  //find latest old note
  int i = 0;
  double minX = std::numeric_limits<double>::max();
  int minIdx = 0;

  // go back a minimum of seqLimit v. the oldest noteOn event
  int adjNotes = 0;
  int j = noteCount;

  //logQ("test", n.x, n.isOn);

  while (noteStream.notes[j].x >= 0 && j >= 0) {
    //logW(LL_CRIT, "break param:",  noteStream.notes[j].x+seqLimit, n.x);
    if (noteStream.notes[j].x + seqLimit <= n.x) {

      if (adjNotes == 0) {
        minX = 0;
      } 
    
      //logW(LL_WARN, "while BREAK on note:", noteStream.notes[j].x, noteStream.notes[j].y);

      break;
    }
    //logW(LL_WARN, "while on note:", noteStream.notes[j].x, noteStream.notes[j].y);

    if (noteStream.notes[j].isOn) {
      i++;
    }

    adjNotes++;
    minX = noteStream.notes[j].x;
    minIdx = j;
    j--;
  }

  if (i != numOn) {
    for (; j >= 0; --j) {
      // avoid this note (which is ON by definition)
      if (i == numOn) {
        // all on notes shifted
        break;
      }
      if (noteStream.notes[j].isOn) {
        if (noteStream.notes[j].x < minX) {
          minX = noteStream.notes[j].x;
          minIdx = j;
        }
        //logQ("j, X", j, noteStream.notes[j].x);
        i++;
      }
    }
  }

  //logQ("MINIDX MINX", minIdx, minX, "|",i,numOn);

  //minIdx = max(minIdx - 5, 0);

  // find track range
  int tr0minY = 127;
  int tr0maxY = 0;
  int tr1minY = 127;
  int tr1maxY = 0;

  // one hand range (approx. a 10th)
  const int handRange = ctr.option.get(optionType::OPTION_HAND_RANGE);
  constexpr int onLimit = 350;

  vector<int> considerN;
  //vector<int> onN;
  for (int j = minIdx; j <= noteCount; j++) {


    if (noteStream.notes[j].isOn || noteStream.notes[j].x+onLimit > n.x) {

      // 1, then 0
      if (j != noteCount) { // exclude this note
        if (noteStream.notes[j].track == 1) {
          tr1maxY = max(tr1maxY, noteStream.notes[j].y);
          tr1minY = min(tr1minY, noteStream.notes[j].y);
        }
        else {
          tr0maxY = max(tr0maxY, noteStream.notes[j].y);
          tr0minY = min(tr0minY, noteStream.notes[j].y);
        }
      }

      //onN.push_back(j);
    }

    considerN.push_back(j);
  }

  //logQ("range:", max(tr0maxY,tr1maxY) -  min(tr0minY,tr1minY));
  //logQ("range0:", tr0maxY - tr0minY);
  //logQ("range1:", tr1maxY - tr1minY);

  // consider range WITH this note

  int new_tr1maxY = max(tr1maxY, n.y);
  int new_tr1minY = min(tr1minY, n.y);
  int new_tr0maxY = max(tr0maxY, n.y);
  int new_tr0minY = min(tr0minY, n.y);
  
  //logQ("range0 (UPD):", new_tr0maxY - new_tr0minY);
  //logQ("range1 (UPD):", new_tr1maxY - new_tr1minY);

  //logQ("consider Y", relevantY);

  if (considerN.size() == 1) {
    if (noteStream.notes[considerN[0]].y >= 60) {
      return 1;
    }
    return 0;
  }

  bool outOfRange0 = new_tr0maxY-new_tr0minY > handRange;
  bool outOfRange1 = new_tr1maxY-new_tr1minY > handRange;

  // force assign to other hand if adding to this track causes an out of range
  if (!outOfRange0 && outOfRange1) {
    return 0;
  }
  else if (outOfRange0 && !outOfRange1) {
    return 1;
  }

  //int tr0sum = 0;
  //int tr1sum = 0;
  int tr0ct = 0;
  int tr1ct = 0;

  for (auto considerNote : considerN) {
    //(noteStream.notes[considerNote].track ? tr1sum : tr0sum) += noteStream.notes[considerNote].y;
    (noteStream.notes[considerNote].track ? tr1ct : tr0ct)++;
  }

  //double tr0avg = ((tr0ct == 0) ? -1 : tr0sum/static_cast<double>(tr0ct));
  //double tr1avg = ((tr1ct == 0) ? -1 : tr1sum/static_cast<double>(tr1ct));
  //double dist0 = fabs(tr0avg - n.y);
  //double dist1 = fabs(tr1avg - n.y);

  //logQ("consider track avg", tr0avg, tr1avg);
 
  // if both in range, choose track with least notes
  if (!outOfRange0 && !outOfRange1) {
    //return tr0ct >= tr1ct;
    //if (fabs(dist0-dist1) < 2) {
      //logW(LL_CRIT, "avg. mediator");
      //return tr0ct >= tr1ct;
    //}
    return noteStream.notes[noteCount-1].track;
  }

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
    //ctr.input.noteStream.buildLineMap();
  }
  else {
    // shift even when midi input is disconnected
    ctr.livePlayOffset += GetFrameTime() * 500;
  }
}
