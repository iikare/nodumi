#include "input.h"

#include <algorithm>
#include <bitset>

#include "controller.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "measure.h"

midiInput::midiInput()
    : midiIn(nullptr),
      msgQueue(0),
      numPort(0),
      curPort(-1),
      noteCount(0),
      numOn(0),
      timestamp(0),
      match(KEYSIG_NONE - 2) {
  midiIn = make_unique<RtMidiIn>();
  if (midiIn == nullptr) {
    logW(LL_WARN, "unable to initialize MIDI input");
  }

  noteStream.setTPQ(DEFAULT_TPQ);
  for (auto& t : noteStream.getTracks()) {
    t.setNoteVector(&noteStream.notes);
  }

  resetStream();
}

void midiInput::initClassifier() {
  if (!classifier_warmup) {
    classifier_warmup = true;
    classifier = make_unique<inputClassifier>();
    classifier->init();
  }
}

void midiInput::reset_lstm_count() { notes_since_lstm_enable = 0; }

void midiInput::terminate() { classifier->terminate(); }

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

void midiInput::resetInput(bool keepPort) {
  ctr.livePlayOffset = 0;
  if (!keepPort) {
    midiIn->closePort();
    curPort = 0;
  }
  msgQueue.clear();
  timestamp = 0;
  noteCount = 0;
  numOn = 0;

  match = vector<pair<int, keySig>>(KEYSIG_NONE - 2);
  ks_type = KEYSIG_C;

  for (int ch = 0; auto& m : match) {
    // exclude 7flat/sharp
    int k_t = KEYSIG_C + ch;
    if (any_of(k_t, KEYSIG_CSHARP, KEYSIG_CFLAT)) {
      ch++;
      k_t++;
    }

    m = make_pair(0, keySig(k_t, 0, 0));
    ch++;
  }

  noteStream.clear();
  noteStream.notes.clear();
  noteStream.measureMap.clear();

  for (unsigned int i = 0; i < noteStream.getTracks().size(); i++) {
    noteStream.getTracks()[i].reset();
  }

  resetStream();
}

void midiInput::resetStream() {
  timeSig ts(4, 4, 0);
  keySig ks(KEYSIG_C, false, 0);
  // ts.setMeasure(0);
  // ks.setMeasure(0);
  measureController m(1, 0, 0, DEFAULT_TPQ, ts, ks);
  m.timeSignatures.push_back(ts);
  m.keySignatures.push_back(ks);
  noteStream.measureMap.push_back(m);

  noteStream.measureMap.back().keySignatures.push_back(ks);
  noteStream.measureMap.back().timeSignatures.push_back(ts);
  noteStream.timeSignatureMap.push_back(make_pair(0, ts));
  noteStream.keySignatureMap.push_back(make_pair(0, ks));
  noteStream.tempoMap.push_back(make_pair(0, DEFAULT_TEMPO));

  noteStream.buildTickSet();

  noteStream.sheetData.reset();
  // noteStream.sheetData.disectMeasure(noteStream.measureMap.back());
  // noteStream.sheetData.findSheetPages();
}

void midiInput::pauseInput() { midiIn->closePort(); }

void midiInput::resumeInput() { openPort(curPort, true); }

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

  if (msgQueue.size() > 0) {
    ctr.output.sendMessage(&msgQueue);
    if (!any_of(static_cast<int>(msgQueue[0]), 248, 254)) {
      // logQ("timestamp:", timestamp);
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
  // logQ(" ");

  // logQ(ctr.livePlayOffset, timestamp);
  if (msgQueue.size()) {
    // if (i) { break; }
    //  logQ(bitset<8>(msgQueue[i]));

    // logQ(ctr.livePlayOffset);

    if (msgQueue[0] == 0b11111000) {  // 248: clock signal
      // cerr << "shift by " << timestamp*100 << endl;
      ctr.livePlayOffset += fmax(0, timestamp) * UNK_CST;
    }
    else if (msgQueue[0] == 0b10010000) {  // 144: note on/off
      if (msgQueue[2] != 0) {              // if note on
        note tmpNote;

        tmpNote.tick =
            ctr.livePlayOffset / UNK_CST * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[1]);
        tmpNote.velocity = static_cast<int>(msgQueue[2]);
        tmpNote.isOn = true;
        tmpNote.measure = noteStream.measureMap.size() - 1;
        tmpNote.number = noteCount;

        // if this is the note on event, duration is undefined
        tmpNote.duration = 1;
        tmpNote.tickDuration = 1;

        // add this note and remove the oldest note in the queue
        constexpr int keysig_match_limit = 32;
        for (auto& m : match) {
          int y_norm = tmpNote.y + 12 * 12;
          int ks_idx = (y_norm + m.second.getIndex()) % 12;
          bool on_key = any_of(ks_idx, 0, 2, 4, 5, 7, 9, 11);
          on_key ? m.first++ : m.first--;

          if (noteCount > keysig_match_limit) {
            y_norm = noteStream.notes[noteCount - 1 - keysig_match_limit].y + 12 * 12;
            ks_idx = (y_norm + m.second.getIndex()) % 12;
            on_key = !any_of(ks_idx, 0, 2, 4, 5, 7, 9, 11);
            on_key ? m.first++ : m.first--;
          }
        }

        ks_type = findKeySig();
        tmpNote.findKeyPos(ks_type);

        tmpNote.findSize(noteStream.getTickSet());

        // partition finder requires the current note to be present
        noteStream.notes.push_back(tmpNote);
        numOn++;

        tmpNote.track =
            ctr.option.get(OPTION::TRACK_DIVISION_LIVE) ? findPartition(tmpNote) : 0;  // by default

        // update track after determination
        noteStream.notes[noteCount].track = tmpNote.track;
        noteStream.measureMap.back().addNote(tmpNote);

        noteStream.getTracks()[tmpNote.track].insert(noteCount);

        // update last index only AFTER track splitter
        noteCount++;

        // cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " <<
        // tmpNote.y << ", " << tmpNote.velocity << endl;

        noteStream.setNoteCount(noteCount);
      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[1]));
        if (idx < noteCount && numOn > 0) {
          noteStream.notes[idx].isOn = false;
          numOn--;
        }
      }

      for (auto& t : noteStream.getTracks()) {
        t.buildChordMap();
      }
      noteStream.buildLineMap();
      // logQ("line size:", noteStream.lines.size());
    }
    else if (msgQueue.size() > 1 && msgQueue[1] == 0b01111001) {
      // reset all controllers
      logW(LL_INFO, "reset event for MIDI port", curPort);

      // resetInput(true);
    }
    else if (msgQueue[0] == 0b11000000) {
      // generic program change event, ignored
    }
    else if (msgQueue[0] == 0b10110000) {
      // generic controller event, ignored
      // events are default sent to output device, no need to process
    }
    else if (msgQueue[0] == 0b11111110) {
      // active sensing event (real-time devices only)
    }
    else {
      string tmp;
      for (long unsigned int i = 0; i < msgQueue.size(); i++) {
        tmp += toHex((int)msgQueue[i]) + " ";
      }
      logQ("unhandled event:", tmp);
    }
  }
}

int midiInput::findPartition(const note& n) {
  // logW(LL_WARN, "new note @", n.y);
  if (classifier->enabled() && notes_since_lstm_enable > classifier->get_length()) {
    if (ctr.option.get(OPTION::USE_LSTM)) {
      vector<int> seq;
      for (int i = 0; i < classifier->get_length(); ++i) {
        int idx = i + 1;
        seq.push_back(noteStream.notes[noteCount - 1 - idx].y);
        seq.push_back(noteStream.notes[noteCount - 1 - idx].track);
      }

      return classifier->run_inference(seq, n.y);
    }
  }
  else {
    notes_since_lstm_enable++;
  }
  return findTrack(n, noteStream);
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
      noteStream.notes[j].tickDuration =
          noteStream.notes[j].duration / UNK_CST * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
      noteStream.notes[j].findSize(noteStream.getTickSet());
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
    double measureLen =
        60.0 / (noteStream.tempoMap.back().second / noteStream.timeSignatureMap.back().second.getQPM()) *
        UNK_CST;
    // logQ(ctr.livePlayOffset, noteStream.measureMap.back().getLocation(), measureLen);

    if (noteStream.sheetData.getDisplayMeasureCount()) {
      noteStream.sheetData.popMeasure();
    }
    noteStream.sheetData.disectMeasure(noteStream.measureMap.back());

    if (ctr.livePlayOffset - noteStream.measureMap.back().getLocation() >= measureLen) {
      int tick = ctr.livePlayOffset / UNK_CST * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
      timeSig ts(4, 4, 0);
      keySig ks(ks_type, false, tick);
      ts.setMeasure(noteStream.measureMap.size());
      ks.setMeasure(noteStream.measureMap.size());
      measureController m(noteStream.measureMap.size(), measureLen * noteStream.measureMap.size(), tick,
                          noteStream.timeSignatureMap[0].second.getQPM() * DEFAULT_TPQ, ts, ks);
      // m.timeSignatures.push_back(ts);
      // m.keySignatures.push_back(ks);

      noteStream.measureMap.push_back(m);
      // logQ("measure size:", noteStream.measureMap.size());

      noteStream.sheetData.disectMeasure(noteStream.measureMap.back());
    }

    if (ctr.renderSheet) {
      noteStream.sheetData.findSheetPages();
    }

    if (midiIn->isPortOpen()) {
      while (updateQueue()) {
        convertEvents();
        updatePosition();
      }
      updatePosition();
      // TODO: consider optimization
      // ctr.input.noteStream.buildLineMap();
    }
    else {
      // shift even when midi input is disconnected
      // only if there's at least one note
      if (noteCount) {
        ctr.livePlayOffset += GetFrameTime() * UNK_CST;
      }
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

int midiInput::findKeySig() {
  auto max_it = std::max_element(match.begin(), match.end(),
                                 [&](const auto& a, const auto& b) { return a.first < b.first; });

  return noteCount ? max_it->second.getKey() : KEYSIG_C;
}

string midiInput::findKeySigLabel() { return keySig(findKeySig(), false, 0).getLabel(); }
