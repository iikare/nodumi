#include "input.h"

#include <algorithm>
#include <bitset>

#include "controller.h"
#include "data.h"
#include "define.h"
#include "log.h"
#include "measure.h"

namespace {

// a live hand is guessed at once, then decided for good when the classifier can see this far past the onset
constexpr double decision_delay = 0.04;

}  // namespace

midiInput::midiInput()
    : midiIn(nullptr),
      msgQueue(0),
      numPort(0),
      curPort(-1),
      noteCount(0),
      numOn(0),
      timestamp(0),
      match(KEYSIG_NONE - 2) {
  midiIn = make_unique<RtMidiIn>(RtMidi::UNSPECIFIED, client_name);
  if (midiIn == nullptr) {
    logW(LL_WARN, "unable to initialize MIDI input");
  }
  classifier = make_unique<inputClassifier>();

  noteStream.setTPQ(DEFAULT_TPQ);
  for (auto& t : noteStream.getTracks()) {
    t.setNoteVector(&noteStream.notes);
  }

  resetStream();
}

void midiInput::initClassifier() {
  if (!classifier_warmup && classifier->enabled()) {
    classifier_warmup = true;
    const auto& model = ctr.getModel("INPUT_CLASS_LIVE");
    classifier->load(CLASSIFIER::LIVE, model.first, model.second);
  }
}

vector<int> midiInput::classify_file(const vector<note>& notes) {
  // the file model is only loaded once a file needs it
  if (!midi_model_tried && classifier->enabled()) {
    midi_model_tried = true;
    const auto& model = ctr.getModel("INPUT_CLASS_FILE");
    classifier->load(CLASSIFIER::MIDI, model.first, model.second);
  }
  return classifier->classify_file(notes);
}

void midiInput::terminate() { classifier->terminate(); }

void midiInput::openPort(int port, bool pauseEvent) {
  if (!pauseEvent) {
    resetInput();
  }

  // ports are numbered as listed in the menu, which leaves out this app's own output
  getPorts();
  if (port < 0 || port >= static_cast<int>(port_map.size())) {
    logW(LL_WARN, "unable to open port number", port);
    return;
  }

  port_name = midiIn->getPortName(port_map[port]);
  midiIn->openPort(port_map[port], port_name);
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
  unassigned.clear();
  undecided.clear();

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
  port_map.clear();
  numPort = midiIn->getPortCount();
  for (int i = 0; i < numPort; i++) {
    // reading this app's own output would loop every message straight back in
    const string name = midiIn->getPortName(i);
    if (name.starts_with(midiOutput::client_name)) {
      continue;
    }
    port_map.push_back(i);
    ports.push_back(name);
  }

  return formatPortName(ports);
}

bool midiInput::updateQueue() {
  int tempTS = midiIn->getMessage(&msgQueue);
  // the queue is drained several times a frame, but the clock only moves once per frame
  if (isUntimedQueue() && !frame_advanced) {
    timestamp += GetFrameTime();
    ctr.livePlayOffset += GetFrameTime() * UNIT_TPS;
    frame_advanced = true;
  }
  else {
    timestamp = tempTS;
  }

  if (msgQueue.size() > 0) {
    // echoing into the port being read from would feed every message straight back in
    if (ctr.output.get_port_name() != port_name) {
      ctr.output.sendMessage(&msgQueue);
    }
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
      ctr.livePlayOffset += fmax(0, timestamp) * UNIT_TPS;
    }
    else if (msgQueue.size() > 2 && any_of(msgQueue[0] & 0xF0, 0x80, 0x90)) {  // note on/off, any channel
      if ((msgQueue[0] & 0xF0) == 0x90 && msgQueue[2] != 0) {                  // if note on
        note tmpNote;

        tmpNote.tick =
            ctr.livePlayOffset / UNIT_TPS * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
        tmpNote.x = ctr.livePlayOffset;
        tmpNote.y = static_cast<int>(msgQueue[1]);
        tmpNote.velocity = static_cast<int>(msgQueue[2]);
        tmpNote.isOn = true;
        tmpNote.measure = noteStream.measureMap.size() - 1;
        tmpNote.number = noteCount;

        // if this is the note on event, duration is undefined
        tmpNote.duration = 0.002 * UNIT_TPS;
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
        noteStream.measureMap.back().addNote(tmpNote);

        if (use_classifier()) {
          // the classifier runs at the end of the frame, once every note of a chord has arrived
          unassigned.push_back(noteCount);
        }
        else {
          add_to_track(noteCount,
                       ctr.option.get(OPTION::TRACK_DIVISION_LIVE) ? findTrack(tmpNote, noteStream) : 0);
        }

        // update last index only AFTER track splitter
        noteCount++;

        // cerr << "this note is: x, Y, Velocity:" << tmpNote.x << ", " <<
        // tmpNote.y << ", " << tmpNote.velocity << endl;

        noteStream.setNoteCount(noteCount);
      }
      else {
        int idx = findNoteIndex(static_cast<int>(msgQueue[1]));
        if (idx >= 0 && numOn > 0) {
          noteStream.notes[idx].isOn = false;
          numOn--;
        }
      }

      // chord and line maps are rebuilt once per frame, not per event
      notes_changed = true;
    }
    else if (msgQueue.size() > 2 && (msgQueue[0] & 0xF0) == 0xB0 && msgQueue[1] == 0b01111001) {
      // reset all controllers
      logW(LL_INFO, "reset event for MIDI port", curPort);

      // resetInput(true);
    }
    else if ((msgQueue[0] & 0xF0) == 0xC0) {
      // generic program change event, ignored
    }
    else if ((msgQueue[0] & 0xF0) == 0xB0) {
      // generic controller event, ignored
      // events are default sent to output device, no need to process
    }
    else if (any_of(msgQueue[0] & 0xF0, 0xA0, 0xD0, 0xE0)) {
      // aftertouch and pitch bend, ignored
    }
    else if (msgQueue[0] >= 0b11111000) {
      // real-time events such as active sensing (real-time devices only)
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

bool midiInput::use_classifier() {
  return ctr.option.get(OPTION::TRACK_DIVISION_LIVE) && ctr.option.get(OPTION::USE_LSTM) &&
         classifier->ready(CLASSIFIER::LIVE);
}

void midiInput::assign_hands() {
  if (unassigned.empty() && undecided.empty()) {
    return;
  }

  // first guesses for this frame's notes, and final decisions for notes old enough to look ahead
  vector<live_query> queries;
  vector<int> waiting;
  for (int idx : unassigned) {
    queries.push_back({idx, (ctr.livePlayOffset - noteStream.notes[idx].x) / UNIT_TPS});
  }
  for (int idx : undecided) {
    const double elapsed = (ctr.livePlayOffset - noteStream.notes[idx].x) / UNIT_TPS;
    if (elapsed >= decision_delay) {
      queries.push_back({idx, elapsed});
    }
    else {
      waiting.push_back(idx);
    }
  }
  if (queries.empty()) {
    return;
  }

  const vector<int> hands =
      use_classifier() ? classifier->classify_live(noteStream.notes, queries) : vector<int>();
  if (hands.size() != queries.size()) {
    // without the classifier, new notes fall back to findTrack and earlier ones keep their first guess
    for (int idx : unassigned) {
      add_to_track(idx, ctr.option.get(OPTION::TRACK_DIVISION_LIVE)
                            ? findTrack(noteStream.notes[idx], noteStream)
                            : 0);
    }
    unassigned.clear();
    undecided.clear();
    return;
  }

  const int current_measure = noteStream.measureMap.size() - 1;
  for (int q = 0; q < static_cast<int>(queries.size()); ++q) {
    const int idx = queries[q].index;
    if (q < static_cast<int>(unassigned.size())) {
      add_to_track(idx, hands[q]);
      if (queries[q].lookahead < decision_delay) {
        waiting.push_back(idx);
      }
    }
    else if (noteStream.notes[idx].measure == current_measure) {
      // a closed measure is already final on the sheet, so its notes keep their first guess
      move_to_track(idx, hands[q]);
    }
  }
  unassigned.clear();
  undecided.swap(waiting);
}

void midiInput::add_to_track(int idx, int hand) {
  noteStream.notes[idx].track = hand;
  noteStream.getTracks()[hand].insert(idx);
}

void midiInput::move_to_track(int idx, int hand) {
  if (noteStream.notes[idx].track == hand) {
    return;
  }
  noteStream.getTracks()[noteStream.notes[idx].track].remove(idx);
  add_to_track(idx, hand);
  notes_changed = true;
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
          noteStream.notes[j].duration / UNIT_TPS * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
      noteStream.notes[j].findSize(noteStream.getTickSet());
      i++;
    }
  }
}

int midiInput::findNoteIndex(int key) {
  // a release belongs to the latest press of that key that is still held
  for (int i = noteCount - 1; i >= 0; i--) {
    if (noteStream.notes[i].y == key && noteStream.notes[i].isOn) {
      return i;
    }
  }
  return -1;
}

void midiInput::update() {
  if (ctr.getLiveState()) {
    double measureLen =
        60.0 / (noteStream.tempoMap.back().second / noteStream.timeSignatureMap.back().second.getQPM()) *
        UNIT_TPS;
    // logQ(ctr.livePlayOffset, noteStream.measureMap.back().getLocation(), measureLen);

    if (noteStream.sheetData.getDisplayMeasureCount()) {
      noteStream.sheetData.popMeasure();
    }
    noteStream.sheetData.disectMeasure(noteStream.measureMap.back());

    if (ctr.livePlayOffset - noteStream.measureMap.back().getLocation() >= measureLen) {
      int tick = ctr.livePlayOffset / UNIT_TPS * (noteStream.tempoMap.back().second * DEFAULT_TPQ / 60.0);
      timeSig ts(4, 4, 0);
      keySig ks(ks_type, false, tick);
      ts.setMeasure(noteStream.measureMap.size());
      ks.setMeasure(noteStream.measureMap.size());
      measureController m(noteStream.measureMap.size(), measureLen * noteStream.measureMap.size(), tick,
                          noteStream.timeSignatureMap[0].second.getQPM() * DEFAULT_TPQ, ts, ks);
      // m.timeSignatures.push_back(ts);
      // m.keySignatures.push_back(ks);

      // held notes continue into the new measure as ties
      for (int j = noteCount - 1, held = 0; j >= 0 && held < numOn; --j) {
        if (noteStream.notes[j].isOn) {
          noteStream.measureMap.back().tie_out_notes.push_back(j);
          m.tie_in_notes.push_back(j);
          held++;
        }
      }

      noteStream.measureMap.push_back(m);
      // logQ("measure size:", noteStream.measureMap.size());

      // the closed measure is final from here on, so redo it with its outgoing ties
      noteStream.sheetData.popMeasure();
      noteStream.sheetData.disectMeasure(noteStream.measureMap[noteStream.measureMap.size() - 2]);
      noteStream.sheetData.disectMeasure(noteStream.measureMap.back());
    }

    if (ctr.renderSheet) {
      noteStream.sheetData.findSheetPages();
    }

    if (midiIn->isPortOpen()) {
      // a flood of input (or a loop back from the output) is spread over frames instead of stalling one
      constexpr int max_events_per_frame = 1024;
      frame_advanced = false;
      for (int processed = 0; processed < max_events_per_frame && updateQueue(); ++processed) {
        convertEvents();
        updatePosition();
      }
      updatePosition();
      assign_hands();

      if (notes_changed) {
        for (auto& t : noteStream.getTracks()) {
          t.buildChordMap();
        }
        noteStream.buildLineMap();
        notes_changed = false;
      }
      // TODO: consider optimization
      // ctr.input.noteStream.buildLineMap();
    }
    else {
      // shift even when midi input is disconnected
      // only if there's at least one note
      if (noteCount) {
        ctr.livePlayOffset += GetFrameTime() * UNIT_TPS;
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
