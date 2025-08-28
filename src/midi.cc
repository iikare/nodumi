#include "midi.h"

#include <algorithm>
#include <queue>
#include <set>
#include <thread>
#include <utility>

#include "data.h"
#include "define.h"
#include "enum.h"
#include "misc.h"
#include "sheetctr.h"
#include "track.h"
#include "track_split.h"

using std::priority_queue;
using std::thread;

int midi::getTempo(int offset) const {
  if (tempoMap.size() != 0 && offset == 0) {
    return tempoMap[0].second;
  }
  if (offset > tempoMap[tempoMap.size() - 1].first) {
    return tempoMap[tempoMap.size() - 1].second;
  }
  for (unsigned int i = 0; i < tempoMap.size(); i++) {
    if (offset > tempoMap[i].first && offset < tempoMap[i + 1].first) {
      return tempoMap[i].second;
    }
  }
  return 120;
}

void midi::buildLineMap() {
  lines.clear();

  auto track_comp = [](const auto& a, const auto& b) { return a.x_l >= b.x_l; };

  if (ctr.getLiveState()) {
    lines.insert(lines.begin(), tracks[0].lines.begin(), tracks[0].lines.end());
    lines.insert(lines.begin(), tracks[1].lines.begin(), tracks[1].lines.end());
    sort(lines.begin(), lines.end(), track_comp);
  }
  else if (tracks.size() > 1) {
    unsigned int n_line = 0;
    for (unsigned int t = 0; t < tracks.size(); ++t) {
      n_line += tracks[t].lines.size();
    }
    // logQ("there are", n_line, "lines to merge");

    lines.reserve(n_line);
    priority_queue<lineData, vector<lineData>, decltype(track_comp)> tt;
    unsigned int idx = 0;

    do {
      for (unsigned int i = 0; i < tracks.size(); i++) {
        if (idx < tracks[i].lines.size()) {
          tt.push(tracks[i].lines[idx]);
        }
      }
      lines.push_back(tt.top());
      tt.pop();
      idx++;
    } while (!tt.empty());
  }
  else if (tracks.size() == 1) {
    lines = tracks[0].lines;
  }
}

void midi::buildTickSet() {
  if (!tpq) {
    logW(LL_CRIT, "MIDI lacks TPQ marker");
    return;
  }

  // logQ("real TPQ:", tpq);

  for (int pos = 0; pos < tickNoteTransformLen; ++pos) {
    // logQ(pos, "maps to TICK VALUE", tpq*tickNoteTransform[pos]);
    tickSet.insert(make_pair(tpq * tickNoteTransform[pos], pos));
  }
}

void midi::buildMessageMap(const MidiFile& mf) {
  // int num_zero = 0;
  vector<pair<double, vector<unsigned char>>> message_vec;
  message_vec.reserve(mf.getEventCount(0));
  for (int i = 0; i < mf.getEventCount(0); i++) {
    if (i && mf[0][i].seconds < mf[0][i - 1].seconds) {
      logW(LL_WARN, "MIDI has nonlinear events");
    }
    // if (mf[0][i].seconds < 0.0001) { num_zero++; }
    // if (mf[0][i].seconds > 0.01){// || mf[0][i].isNote()) {
    if (mf[0][i].isNote() || mf[0][i].isController()) {
      // if (1) {
      message_vec.push_back(make_pair(mf[0][i].seconds * UNK_CST, static_cast<vector<unsigned char>>(mf[0][i])));
    }
  }
  message_vec.shrink_to_fit();
  message = multiset<pair<double, vector<unsigned char>>>(message_vec.begin(), message_vec.end());
  message_vec.clear();

  // logQ(num_zero, "events at position < 0.0001s");
}

int midi::findMeasure(int offset) const {
  if (offset <= 0) {
    // logQ("bad offset", offset);
    return 0;
  }
  if (measureMap[measureMap.size() - 1].getLocation() < offset) {
    return measureMap.size();
  }
  // requires a built measure map
  for (unsigned int i = 0; i < measureMap.size() - 1; i++) {
    if (measureMap[i].getLocation() <= offset && measureMap[i + 1].getLocation() > offset) {
      return i + 1;
    }
  }
  // logW(LL_WARN, "invalid current measure at offset", offset);
  return measureMap.size();
}

void midi::addTimeSignature(double position, const timeSig& timeSignature) {
  if (timeSignatureMap.size() != 0 && timeSignatureMap[timeSignatureMap.size() - 1].second == timeSignature) {
    return;
  }
  timeSignatureMap.push_back(make_pair(position, timeSignature));
}

void midi::addKeySignature(double position, const keySig& keySignature) {
  if (keySignatureMap.size() != 0 && keySignatureMap[keySignatureMap.size() - 1].second == keySignature) {
    return;
  }
  keySignatureMap.push_back(make_pair(position, keySignature));
}

keySig midi::eventToKeySignature(int keySigType, bool isMinor, int tick) {
  int keyType = KEYSIG_NONE;

  switch (keySigType) {
    case 0:
      keyType = KEYSIG_C;
      break;
    case 1:
      keyType = KEYSIG_G;
      break;
    case 2:
      keyType = KEYSIG_D;
      break;
    case 3:
      keyType = KEYSIG_A;
      break;
    case 4:
      keyType = KEYSIG_E;
      break;
    case 5:
      keyType = KEYSIG_B;
      break;
    case 6:
      keyType = KEYSIG_FSHARP;
      break;
    case 7:
      keyType = KEYSIG_CSHARP;
      break;
    case 255:
      keyType = KEYSIG_F;
      break;
    case 254:
      keyType = KEYSIG_BFLAT;
      break;
    case 253:
      keyType = KEYSIG_EFLAT;
      break;
    case 252:
      keyType = KEYSIG_AFLAT;
      break;
    case 251:
      keyType = KEYSIG_DFLAT;
      break;
    case 250:
      keyType = KEYSIG_GFLAT;
      break;
    case 249:
      keyType = KEYSIG_CFLAT;
      break;
  }

  return keySig(keyType, isMinor, tick);
}

int midi::findKeySig() {
  vector<pair<int, keySig>> match(KEYSIG_NONE - 2);

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

  for (auto& m : match) {
    for (const auto& n : notes) {
      int y_norm = n.y + 12 * 12;
      int ks_idx = (y_norm + m.second.getIndex()) % 12;

      bool on_key = any_of(ks_idx, 0, 2, 4, 5, 7, 9, 11);
      on_key ? m.first++ : m.first--;
    }
  }

  // for (const auto& m : match) { logQ(m.first, "-", m.second.getLabel()); }
  auto max_it =
      std::max_element(match.begin(), match.end(), [&](const auto& a, const auto& b) { return a.first < b.first; });
  return max_it->second.getKey();
  // return KEYSIG_C;
}

timeSig midi::getTimeSignature(double offset) {
  timeSig timeSignature = {0, -1, 1};
  for (unsigned int i = 0; i < timeSignatureMap.size(); i++) {
    if (offset > timeSignatureMap[i].first && offset < timeSignatureMap[i + 1].first) {
      return timeSignatureMap[i].second;
    }
  }
  return timeSignature;
}

keySig midi::getKeySignature(double offset) {
  keySig keySignature = {0, 1, -1};
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (offset > keySignatureMap[i].first && offset < keySignatureMap[i + 1].first) {
      return keySignatureMap[i].second;
    }
  }
  return keySignature;
}

void midi::linkKeySignatures() {
  if (keySignatureMap.size() == 0) {
    return;
  }
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (i == 0) {
      continue;
    }
    keySignatureMap[i].second.setPrev(&keySignatureMap[i - 1].second);
  }
}

void midi::clear() {
  notes.clear();
  message.clear();
  lines.clear();
  tempoMap.clear();
  tracks.clear();
  trackHeightMap.clear();
  measureMap.clear();
  timeSignatureMap.clear();
  keySignatureMap.clear();

  tickSet.clear();
  itemStartSet.clear();

  sheetData.reset();

  velocityBounds = make_pair(127, 0);

  noteCount = 0;
  trackCount = 0;
  tpq = 0;

  lastTime = 0;
  lastTick = 0;
}

void midi::load(string file, stringstream& buf) {
  buf.str("");
  buf.clear();
  ifstream midiData(file, std::ios_base::in | std::ios_base::binary);
  buf << midiData.rdbuf();
  midiData.close();

  load(buf);
}

void midi::load(stringstream& buf) {
  MidiFile midifile(buf);
  if (!midifile.status()) {
    logW(LL_WARN, "invalid MIDI file");
    return;
  }

  clear();

  midifile.linkNotePairs();
  midifile.doTimeAnalysis();

  trackCount = midifile.getTrackCount();
  tracks.resize(trackCount);
  for (auto& t : tracks) {
    t.setNoteVector(&notes);
  }

  tpq = midifile.getTicksPerQuarterNote();
  lastTime = midifile.getFileDurationInSeconds() * UNK_CST;
  lastTick = midifile.getFileDurationInTicks();

  buildTickSet();

  vector<pair<double, int>> trackInfo;

  for (int i = 0; i < trackCount; i++) {
    bool once = true;
    for (int j = 0; j < midifile.getEventCount(i); j++) {
      if (midifile[i][j].isNoteOn()) {
        if (once) {
          once = false;
          trackInfo.push_back(make_pair(midifile[i][j].seconds * UNK_CST, i));
        }
        noteCount++;
      }
    }
  }

  if (noteCount == 0) {
    logW(LL_WARN, "zero length MIDI file");
    return;
  }

  // sort(trackInfo.begin(), trackInfo.end());
  notes.resize(noteCount);
  int idx = 0;

  for (unsigned int track = 0; track < trackInfo.size(); track++) {
    int i = trackInfo[track].second;
    for (int j = 0; j < midifile.getEventCount(i); j++) {
      if (midifile[i][j].isNoteOn()) {
        notes[idx].number = idx;
        notes[idx].tick = midifile[i][j].tick;
        notes[idx].tickDuration = midifile[i][j].getTickDuration();
        notes[idx].track = i;
        notes[idx].duration = midifile[i][j].getDurationInSeconds() * UNK_CST;
        notes[idx].x = midifile[i][j].seconds * UNK_CST;
        notes[idx].y = midifile[i][j].getKeyNumber();
        notes[idx].velocity = midifile[i][j][2];

        velocityBounds.first = min(notes[idx].velocity, velocityBounds.first);
        velocityBounds.second = max(notes[idx].velocity, velocityBounds.second);

        notes[idx].findSize(tickSet);
        tracks.at(notes[idx].track).insert(idx);
        idx++;
      }
    }
  }

  tracks.erase(remove_if(tracks.begin(), tracks.end(), [&](auto& tr) { return !tr.getNoteCount(); }), tracks.end());

  if (ctr.option.get(OPTION::TRACK_DIVISION_MIDI) && trackCount == 1) {
    logW(LL_INFO, "MIDI track division enabled - performing division");

    trackCount = 2;
    tracks.push_back(tracks[0]);
    trackInfo.push_back(trackInfo[0]);
    trackInfo[1].second = 1;
    for (auto& t : tracks) {
      t.reset();
    }

    for (int idx = 0; auto& n : notes) {
      n.track = findTrack(n, *this, false, idx);
      // logQ(n.track);
      // logQ(tracks.at(n.track).getNoteCount());
      tracks.at(n.track).insert(idx);
      idx++;
    }
  }

  midifile.joinTracks();
  midifile.sortTracks();

  thread message_thread = thread(&midi::buildMessageMap, this, std::ref(midifile));
  // buildMessageMap(midifile);

  for (int i = 0; i < midifile.getEventCount(0); i++) {
    if (midifile[0][i].isTempo()) {
      tempoMap.push_back(make_pair(midifile[0][i].seconds * UNK_CST, midifile[0][i].getTempoBPM()));
    }
    if (midifile[0][i].isTimeSignature()) {
      // logQ(LL_INFO, "time sig at event", j);
      addTimeSignature(midifile[0][i].seconds * UNK_CST,
                       {(int)midifile[0][i][3], (int)pow(2, (int)midifile[0][i][4]), midifile[0][i].tick});
    }
    if (midifile[0][i].isKeySignature()) {
      // logQ(LL_INFO, "key sig at event", i);
      keySig tmpKS = eventToKeySignature((int)midifile[0][i][3], (bool)midifile[0][i][4], midifile[0][i].tick);
      addKeySignature(midifile[0][i].seconds * UNK_CST, tmpKS);
    }
  }

  // link keysigs with left-adjacents
  linkKeySignatures();

  for (unsigned int i = 0; i < tracks.size(); i++) {
    // build track height map
    trackHeightMap.push_back(make_pair(i, tracks[i].getAverageY()));
  }

  vector<thread> track_thread;
  vector<int> st_track_idx;

  for (unsigned int i = 0; i < tracks.size(); i++) {
    // build track chord map (and implicitly, line map as well)
    if (tracks[i].getNoteCount() > 1000) {
      track_thread.emplace_back([&](trackController& f) { f.buildChordMap(); }, std::ref(tracks[i]));
      // logQ("#, S: {", i, tracks[i].getNoteCount(), "}");
    }
    else {
      // tracks[i].buildChordMap();
      st_track_idx.push_back(i);
    }
    // logQ("THREAD ID:", track_thread[i].get_id());
    // tracks[i].buildChordMap();
  }

  for (auto& i : st_track_idx) {
    tracks[i].buildChordMap();
  }

  for (auto& t : track_thread) {
    t.join();
  }

  sort(trackHeightMap.begin(), trackHeightMap.end(),
       [](const pair<int, double>& left, const pair<int, double>& right) { return left.second < right.second; });

  // build measure map
  if (timeSignatureMap.size() == 0) {
    logW(LL_INFO, "no time signatures detected, adding default time signature");
    timeSignatureMap.push_back(make_pair(0, timeSig(4, 4, 0)));
  }
  if (keySignatureMap.size() == 0) {
    int detect_ks_t = findKeySig();
    keySig detect_ks = keySig(detect_ks_t, 0, 0);

    logW(LL_INFO, "no key signatures detected, adding closest match -", detect_ks.getLabel());

    // keySignatureMap.push_back(make_pair(0,keySig(KEYSIG_C,0,0)));
    keySignatureMap.push_back(make_pair(0, detect_ks));
  }

  int cTick = 0;
  timeSig cTimeSig = timeSignatureMap[0].second;
  keySig cKeySig = keySignatureMap[0].second;
  idx = 0;
  int idxK = 0;

  int measureNum = 1;

  measureMap.reserve(4 * lastTick / (cTimeSig.getQPM() * tpq));
  measureMap.push_back(measureController(measureNum++, 0, 0, cTimeSig.getQPM() * tpq, cTimeSig, cKeySig));
  while (cTick < lastTick) {
    cTick += cTimeSig.getQPM() * tpq;

    if (idxK + 1 < static_cast<int>(keySignatureMap.size())) {
      if (cTick >= keySignatureMap[idxK + 1].second.getTick()) {
        cKeySig = keySignatureMap[++idxK].second;
      }
    }

    if (idx + 1 < static_cast<int>(timeSignatureMap.size())) {
      if (cTick >= timeSignatureMap[idx + 1].second.getTick()) {
        cTimeSig = timeSignatureMap[++idx].second;
      }
    }
    // logQ(measureNum, "to",cKeySig.getAcc());
    measureMap.push_back(measureController(measureNum++, midifile.getTimeInSeconds(cTick) * UNK_CST, cTick,
                                           cTimeSig.getQPM() * tpq, cTimeSig, cKeySig));
  }
  measureMap.pop_back();
  measureMap.shrink_to_fit();

  // auto itemStartCmp = [] (pair<int, int> a, pair<int, int> b) { return
  // a.first <= b.first; }; set<pair<int, int>, decltype(itemStartCmp)>
  // itemStartSet = {};
  for (int m = 0; auto& measure : measureMap) {
    // logQ(measure.getTick());
    itemStartSet.insert(make_pair(measure.getTick(), m++));
    // measure.clear();
  }

  for (auto& note : notes) {
    auto mIt = itemStartSet.lower_bound(make_pair(note.tick, 0));
    // measures have 0-based index, but 1-based for rendering
    int noteMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    // logQ(note.tick, "has closest measure start", noteMeasure);
    note.measure = noteMeasure;
    measureMap[noteMeasure].addNote(note);
  }

  for (auto& ts : timeSignatureMap) {
    auto mIt = itemStartSet.lower_bound(make_pair(ts.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int tsMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    // logQ("ts", ts.second.getTick(), "has closest measure start",
    // 1+tsMeasure); logQ("ts", ts.second.getTick(), "has value",
    // ts.second.getTop(), ts.second.getBottom());
    ts.second.setMeasure(tsMeasure);
    measureMap[tsMeasure].timeSignatures.push_back(ts.second);
  }

  for (auto& ks : keySignatureMap) {
    auto mIt = itemStartSet.lower_bound(make_pair(ks.second.getTick(), 0));
    // measures have 0-based index, but 1-based for rendering
    int ksMeasure = (mIt != itemStartSet.begin() ? (--mIt)->second : 0);

    // logQ("ks", ks.second.getAcc(),"@", ks.second.getTick(), "has closest
    // measure start", 1+ksMeasure);
    ks.second.setMeasure(ksMeasure);
    measureMap[ksMeasure].keySignatures.push_back(ks.second);
  }

  // create sheet music position data
  for (/*int z = 0;*/ auto& measure : measureMap) {
    // logQ("measure",z+1,"at tick",measure.getTick());
    // if (measure.keySignatures.size() > 0) {
    // logQ("measure",z+1,"has INSIDE",measure.keySignatures[0].getAcc(),
    // measure.keySignatures[0].getTick());
    //}
    // logQ("measure",z+1,"has keysig",measure.currentKey.getAcc());
    // logQ("measure",z+1,"has timesig",measure.currentTime.getTop(),
    // measure.currentTime.getBottom());

    // z++;

    for (auto& note : measure.notes) {
      // map note position to key (only if on/off signature)
      note->findKeyPos(measure.currentKey);
    }
    sheetData.disectMeasure(measure);
  }

  sheetData.findSheetPages();
  // for (int m = 0; auto& measure : measureMap) {
  // logQ(measure.notes.size(), "notes in measure", 1+m++);
  //}

  // build line vertex map
  buildLineMap();

  // lastTime = notes[getNoteCount() - 1].x + notes[getNoteCount() -
  // 1].duration; logII(LL_CRIT, (midifile.getFileDurationInTicks()) / (tpq * 4)
  // + 1); logII(LL_CRIT, measureMap.size());

  // logQ("total ks", keySignatureMap.size());

  message_thread.join();
}
