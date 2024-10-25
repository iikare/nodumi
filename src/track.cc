#include "track.h"

#include <string>

#include "define.h"

void trackController::reset() {
  note_idx.clear();
  chords.clear();
  lines.clear();
  noteCount = 0;
  noteSum = 0;
}

void trackController::insert(unsigned int n) {
  noteCount++;
  noteSum += n_vec->at(n).y;
  note_idx.push_back(n);
}

void trackController::buildChordMap() {
  chords.clear();
  // requires notes to be added first already in note_idx
  // logQ("track has", note_idx.size(), "notes");
  if (note_idx.size() == 0) {
    return;
  }

  for (unsigned int n = 0; n < note_idx.size(); ++n) {
    if (n != 0 && n_vec->at(note_idx[n]).x < n_vec->at(note_idx[n] - 1).x) {
      logQ("MISALIGNED @ idx:", note_idx[n], "but", n);
    }

    chords.push_back({note_idx[n]});
    unsigned int c_chord = chords.size() - 1;

    while (n + 1 < note_idx.size()) {
      if (n_vec->at(note_idx[n]).x == n_vec->at(note_idx[n + 1]).x) {
        chords[c_chord].add_member(note_idx[n + 1]);
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
  // for (unsigned int c = 0; c+1 < chords.size(); ++c) {

  // if (n_vec->at(chords[c].data()[0]).x > n_vec->at(chords[c+1].data()[0]).x)
  // { logQ("WARN: chords", c, c+1, "not created in sorted order");
  // logQ(n_vec->at(chords[c].data()[0]).x ,
  // n_vec->at(chords[c+1].data()[0]).x);
  //}

  //}

  buildLineMap();

  // logQ("avg. chord size:",
  // static_cast<double>(note_idx.size())/chords.size()); logQ("generated",
  // lines.size(), "lines");
}

void trackController::buildLineMap() {
  lines.clear();
  for (unsigned int c_chord = 0; c_chord < chords.size(); ++c_chord) {
    if (c_chord + 1 == chords.size()) {
      buildLine(c_chord, c_chord);
    }
    else {
      buildLine(c_chord, c_chord + 1);
    }
  }
}

void trackController::buildLine(unsigned int l, unsigned int r) {
  auto chord_l = chords[l].data();
  auto chord_r = chords[r].data();

  // if (chord_l.size() > 1) {
  // for (unsigned int idx_l = 0; idx_l < chord_l.size(); ++idx_l) {
  // if (idx_l + 1 < chord_l.size()) {
  // if (n_vec->at(chord_l[idx_l]).y > n_vec->at(chord_l[idx_l+1]).y) {
  // logQ("CHORD", l, "OF SIZE", chord_l.size(), "NOT SORTED");
  //}
  //}
  //}
  //}

  double l_start = n_vec->at(chord_l[0]).x;
  int l_end_note = chords[l].latest_end(n_vec);
  double l_duration = n_vec->at(l_end_note).duration;
  double r_start = n_vec->at(chord_r[0]).x;
  bool in_progress = n_vec->at(chord_l[0]).isOn;

  // unsigned int n_line = 0;

  // only link spatially near notes
  if (l == r || l_start + 2 * l_duration < r_start) {
    for (unsigned int n_l = 0; n_l < chord_l.size(); ++n_l) {
      const auto& l_note = n_vec->at(chord_l[n_l]);
      lines.push_back({chord_l[n_l], l_note.x, l_note.y, l_note.x + l_note.duration, l_note.y, in_progress});
    }
    return;
  }

  auto push_verts = [&](unsigned int n_l, unsigned int n_r) {
    const auto& l_note = n_vec->at(chord_l[n_l]);
    const auto& r_note = n_vec->at(chord_r[n_r]);

    // if (r_note.x < l_note.x) {
    // logQ("NEGATIVE LINE OFFSET");
    // return;
    //}
    // n_line++;

    constexpr int line_max_height = 32;
    int line_end_y = r_note.y;

    if (abs(l_note.y - r_note.y) > line_max_height) {
      // logQ(abs(l_note.y-r_note.y));
      line_end_y = l_note.y;
    }

    lines.push_back({chord_l[n_l], l_note.x, l_note.y, r_note.x, line_end_y,
                     // r_note.y,
                     in_progress});
  };

  auto get_y_l = [&](unsigned int idx) { return n_vec->at(chord_l[idx]).y; };
  auto get_y_r = [&](unsigned int idx) { return n_vec->at(chord_r[idx]).y; };

  if (chord_l.size() == chord_r.size()) {
    // equal size
    for (unsigned int n_l = 0; n_l < chord_l.size(); ++n_l) {
      push_verts(n_l, n_l);
    }
  }
  else if (chord_l.size() < chord_r.size()) {
    // left side smaller
    unsigned int dup = chord_r.size() - chord_l.size();
    unsigned int n_l = 0;
    unsigned int n_r = 0;

    while (n_r < chord_r.size()) {
      if (dup > 0) {
        // can choose between left endpoints
        if (n_l + 1 < chord_l.size()) {
          bool use_current = abs(get_y_l(n_l) - get_y_r(n_r)) < abs(get_y_l(n_l + 1) - get_y_r(n_r));

          if (use_current) {
            dup--;
          }
          else {
            n_l++;
          }
        }
      }
      else if (n_l + 1 < chord_l.size()) {
        // must choose next endpoint
        n_l++;
      }

      push_verts(n_l, n_r);
      n_r++;
    }

    // unsigned int n_l = 0;
    // for (n_l = 0; n_l < chord_l.size(); ++n_l) {
    // push_verts(n_l, n_l);
    //}
    // n_l--;
    // for (unsigned int n_r = n_l+1; n_r < chord_r.size(); ++n_r) {
    // push_verts(n_l, n_r);
    //}
  }
  else {
    // right side smaller
    unsigned int dup = chord_l.size() - chord_r.size();
    unsigned int n_l = 0;
    unsigned int n_r = 0;

    while (n_l < chord_l.size()) {
      if (dup > 0) {
        // can choose between right endpoints
        if (n_r + 1 < chord_r.size()) {
          bool use_current = abs(get_y_l(n_l) - get_y_r(n_r)) < abs(get_y_l(n_l) - get_y_r(n_r + 1));

          if (use_current) {
            dup--;
          }
          else {
            n_r++;
          }
        }
        else {
          n_r = chord_r.size() - 1;
        }
      }
      else if (n_r + 1 < chord_r.size()) {
        // must choose next endpoint
        n_r++;
      }

      push_verts(n_l, n_r);
      n_l++;
    }

    // for (unsigned int n_l = 0; n_l < chord_l.size(); ++n_l) {
    // push_verts(n_l, (n_l < chord_r.size()) ? n_l : chord_r.size()-1);
    //}
  }

  // unsigned int r_line = max(chord_l.size(), chord_r.size());

  // if (r_line > n_line) {
  // logQ(l, "LESS: need", r_line, "got", n_line);

  //}
  // if (r_line < n_line) {
  // logQ(l, "MORE: need", r_line, "got", n_line);
  //}

  // if (r_line != n_line) {

  // if (chord_l.size() > chord_r.size()) {
  // logQ("LEFT LARGE");
  //}
  // else if (chord_l.size() < chord_r.size()) {
  // logQ("RIGHT LARGE");

  //}
  // else {
  // logQ("EQUAL");
  //}
  //}
}
