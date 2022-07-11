#include "sheetcomp.h"
#include "note.h"
#include <type_traits>



void sheetMeasure::addSpace(int space) {
  spacing.push_back(space);
}
    
void sheetChord::addNote(note& n) {

  auto it = chord.find(make_pair(n.tick, vector<note*>{}));
  if (it != chord.end()) {
    // add to chord
    auto vec = const_cast<vector<note*>*>(&(it->second));
    vec->push_back(&n);
  }
  else {
    pair<int, vector<note*>> newPos = make_pair(n.tick, vector<note*>{&n});
    chord.insert(newPos);
  }
}
