#include <string>
#include "track.h"
#include "define.h"

using std::pair;

void trackController::insert(int idx, note* newNote) {
  if (newNote == nullptr) {
    logW(LL_CRIT, "empty note passed to trackController::insert with index", idx);
    return;
  }
  if (tail != nullptr && tail->x > newNote->x) {
    logW(LL_WARN, "mismatched note on track", newNote->track, "(", tail->x, "→", newNote->x, ")");
  }
  if (head == nullptr) {
    head = newNote;
  }
  if (tail == nullptr) {
    tail = newNote;
  }
  else if (tail->x == newNote->x) {
    tail->chordNext = newNote;
    newNote->prev = tail;
  }
  else if (!ctr.getLiveState()){
    // find chord root
    note* p = tail;
    while (p->prev != nullptr && p->prev->x == p->x) {
      p = p->prev;
    }
    p->next = newNote;
    newNote->prev = p;
  }
  
  tail = newNote;

  noteIdxMap.insert(pair<int, note*>(idx, newNote));
  noteCount++;
  noteSum+= newNote->y;
}

void trackController::reset() {
  head = nullptr;
  tail = nullptr;
  noteCount = 0;
  noteSum = 0;
  noteIdxMap.clear();
  noteIdxMap = {};

  note_idx.clear();
  chords.clear();
  lines.clear();
}

note* trackController::getLastNote() {
  if (noteIdxMap.size() == 0) {
    return nullptr;
  }
  return noteIdxMap[noteIdxMap.size() - 1];
}

void trackController::fixLastNote() {
  if (!getLastNote()) {
    return;
  }
  getLastNote()->isLastOnTrack = true;
}


void trackController::insert(unsigned int n) {
  note_idx.push_back(n);
}

void trackController::buildChordMap() {
  // requires notes to be added first already in note_idx
  //logQ("track has", note_idx.size(), "notes");
  if (note_idx.size() == 0) {
    return;
  }

  for (unsigned int n = 0; n < note_idx.size(); ++n) {
    if (n !=0 && n_vec->at(note_idx[n]).x < n_vec->at(note_idx[n]-1).x) {
      logQ("MISALIGNED @ idx:", note_idx[n], "but", n);
    }

    chords.push_back({note_idx[n]});
    unsigned int c_chord = chords.size()-1;
    
    while (n+1 < note_idx.size()) {
      if (n_vec->at(note_idx[n]).x == n_vec->at(note_idx[n+1]).x) {
        chords[c_chord].add_member(note_idx[n+1]);
      }
      else {
        break;
      }
      n++;
    }
  }

  // not guaranteed to be in increasing order of y
  for (auto& c : chords) {
    c.sort_asc(n_vec);
  }
  for (unsigned int c = 0; c+1 < chords.size(); ++c) {

    if (n_vec->at(chords[c].data()[0]).x > n_vec->at(chords[c+1].data()[0]).x) {
      logQ("WARN: chords", c, c+1, "not created in sorted order");
      logQ(n_vec->at(chords[c].data()[0]).x , n_vec->at(chords[c+1].data()[0]).x);
    }

  }

  buildLineMap();

  //logQ("avg. chord size:", static_cast<double>(note_idx.size())/chords.size());

  logQ("generated", lines.size(), "lines");


}

void trackController::buildLineMap() {
  for (unsigned int c_chord = 0; c_chord < chords.size(); ++c_chord) {
    if (c_chord+1 == chords.size()) {
      buildLine(c_chord, c_chord);
    }
    else {
      buildLine(c_chord, c_chord+1);
    }
  }
}

void trackController::buildLine(unsigned int l, unsigned int r) {

  auto chord_l = chords[l].data();
  auto chord_r = chords[r].data();

  //if (chord_l.size() > 1) {
    //for (unsigned int idx_l = 0; idx_l < chord_l.size(); ++idx_l) {
      //if (idx_l + 1 < chord_l.size()) {
        //if (n_vec->at(chord_l[idx_l]).y > n_vec->at(chord_l[idx_l+1]).y) {
          //logQ("CHORD", l, "OF SIZE", chord_l.size(), "NOT SORTED");
        //}
      //}
    //}
  //}
 
  double l_start = n_vec->at(chord_l[0]).x;    
  int l_end_note = chords[l].latest_end(n_vec);
  double l_duration = n_vec->at(l_end_note).duration; 
  double r_start = n_vec->at(chord_r[0]).x;    

  // only link spatially near notes
  if (l==r || l_start + 2*l_duration < r_start) {
    for (unsigned int n_l = 0; n_l < chord_l.size(); ++n_l) {
      const auto& l_note = n_vec->at(chord_l[n_l]);
      lines.push_back({chord_l[n_l],
                       l_note.x,
                       l_note.y,
                       l_note.x+l_note.duration,
                       l_note.y});
    }
    return;
  }

  auto push_verts = [&](unsigned int n_l, unsigned int n_r) {
    const auto& l_note = n_vec->at(chord_l[n_l]);
    const auto& r_note = n_vec->at(chord_r[n_r]);
    lines.push_back({chord_l[n_l],
                     l_note.x,
                     l_note.y,
                     r_note.x,
                     r_note.y});
  };

  if (chord_l.size() == chord_r.size()) {
    // equal size
    for (unsigned int n_l = 0; n_l < chord_l.size(); ++n_l) {
      push_verts(n_l, n_l);
    }
  }
  else if (chord_l.size() < chord_r.size()) {
    // left side smaller
    unsigned int n_l = 0;
    for (; n_l < chord_l.size(); ++n_l) {
      push_verts(n_l, n_l);
    }
    n_l--;
    for (unsigned int n_r = n_l; n_r < chord_r.size(); ++n_r) {
      push_verts(n_l, n_r);
    }

  }
  else {
    // right side smaller
    unsigned int n_r = 0;
    for (; n_r < chord_r.size(); ++n_r) {
      push_verts(n_r, n_r);
    }
    n_r--;
    for (unsigned int n_l = n_r; n_l < chord_l.size(); ++n_l) {
      push_verts(n_r, n_l);
    }
  }


  //vector<int> linePos = {};

  //note& pNow = n_vec->at(note_idx[l]);
  //note& pNext = n_vec->at(note_idx[r]);
  //bool pushLine = false;

  //auto pushVerts = [&] {
      //linePos.push_back(pNow.number);
      
      //linePos.push_back(pNow.x);
      //linePos.push_back(pNow.y);
      
      //if (!pushLine) {
        //linePos.push_back(pNext.x);
        //linePos.push_back(pNext.y);
      //}
      //else {
        //linePos.push_back(pNow.x + pNow.duration);
        //linePos.push_back(pNow.y);
      //}
  //};

  //// only link spatially near notes
  //if (now.x + 2 * now.duration < next.x) {
    //pushLine = true;
  //}


  //if (now.getChordSize() == next.getChordSize()) {
    //for (int i = 0; i < now.getChordSize(); i++) {
      //pushVerts();
      //pNow = pNow.chordNext;
      //pNext = pNext.chordNext;
    //}
  //}
  //else if (now.getChordSize() > next.getChordSize()) {
    //for (int i = 0; i < next.getChordSize(); i++) {
      //pushVerts();
      //pNow = pNow.chordNext;
      //if (i != next.getChordSize() - 1) {
        //pNext = pNext.chordNext;
      //}
    //}
    //for (int i = 0; i < now.getChordSize() - next.getChordSize(); i++) {
      //pushVerts();
      //pNow = pNow.chordNext;
    //}
  //}
  //else {
    //for (int i = 0; i < now.getChordSize(); i++) {
      //pushVerts();
      //pNext = pNext.chordNext;
      //if (i != now.getChordSize() - 1) {
        //pNow = pNow.chordNext;
      //}
    //}
    //for (int i = 0; i < next.getChordSize() - now.getChordSize(); i++) {
      //pushVerts();
      //pNext = pNext.chordNext;
    //}
  //}

}
