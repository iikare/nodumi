#include<algorithm>
#include<bitset>
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
        lastOnIdx = noteCount;
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

  // first, determine if this is a new sequence
  constexpr int seqLimit = 2000;
  if (lastOnIdx == -1 || ctr.livePlayOffset - noteStream.notes[lastOnIdx].x > seqLimit) {
    // new sequence, reset parameters
   

    logW(LL_CRIT, "new seq", n.x);

    //if (n.y >= 60) {
      //return 1;
    //}
    //return 0;
  }

  logW(LL_WARN, "new note @", n.y);


  // else, use current sequence parameter to determine
  //find latest old note
  int i = 0;
  double minX = std::numeric_limits<double>::max();
  int minIdx = 0;


  // go back a minimum of seqLimit v. the oldest noteOn event
  if (true){//numOn == 0) {
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
          logQ("j, X", j, noteStream.notes[j].x);
          i++;
        }
      }
    }
     
  }
  //else {
    //for (int j = noteCount - 1; j >= 0; --j) {
      //// avoid this note (which is ON by definition)
      //if (i == numOn) {
        //// all on notes shifted
        //break;
      //}
      //if (noteStream.notes[j].isOn) {
        //if (noteStream.notes[j].x < minX) {
          //minX = noteStream.notes[j].x;
          //minIdx = j;
        //}
        //logQ("j, X", j, noteStream.notes[j].x);
        //i++;
      //}
    //}
  //}

  logQ("MINIDX MINX", minIdx, minX, "|",i,numOn);

  //minIdx = max(minIdx - 5, 0);

  // find span range
  int minY = 127;
  int maxY = 0;

  // one hand range (approx. a 10th)
  constexpr int handRange = 16;

  vector<int> relevantY;
  vector<int> considerN;
  for (int j = minIdx; j <= noteCount; j++) {

    maxY = max(maxY, noteStream.notes[j].y);
    minY = min(minY, noteStream.notes[j].y);


    considerN.push_back(j);
    relevantY.push_back(noteStream.notes[j].y);

  }



  logQ("range:", minY, maxY);
  logQ("consider Y", relevantY);

  //constexpr int maxRange = 16; // tenth range

  //if (maxY-minY <= maxRange) {
  
    //// use earliest position
    //if (noteStream.notes[minIdx].y >= 60) {
      //return 1;
    //}
    //else { 
      //return 0;
    //}
  //}

  //else {//if (relevantY.size() == 1) {
    //if (y >= 60) {
      //return 1;
    //}
    //return 0;
  //}
  ////else {

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

  if (relevantY.size() == 1) {
    if (relevantY[0] >= 60) {
      return 1;
    }
    return 0;
  }


  if (maxY-minY <= handRange) {    
    return noteStream.notes[noteCount-1].track;
  }


  int tr0sum = 0;
  int tr1sum = 0;
  int tr0ct = 0;
  int tr1ct = 0;

  for (auto considerNote : considerN) {
    (noteStream.notes[considerNote].track ? tr1sum : tr0sum) += noteStream.notes[considerNote].y;
    (noteStream.notes[considerNote].track ? tr1ct : tr0ct)++;
  }

  double tr0avg = ((tr0ct == 0) ? -1 : tr0sum/static_cast<double>(tr0ct));
  double tr1avg = ((tr1ct == 0) ? -1 : tr1sum/static_cast<double>(tr1ct));


  logQ("consider track avg", tr0avg, tr1avg);


  if (tr0avg != -1 && tr1avg != -1) {
    int dist0 = fabs(tr0avg - n.y);
    int dist1 = fabs(tr1avg - n.y);

    return (dist0 >= dist1) ? 1 : 0;

  }
  else {

    if (n.y >= 60) {
      return 1;
    }
    return 0;

    if (tr0avg == -1) {
      return 0;
    }
    return 1;
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
    //ctr.liveInput.noteStream.buildLineMap();
  }
  else {
    // shift even when midi input is disconnected
    ctr.livePlayOffset += GetFrameTime() * 500;
  }
}
