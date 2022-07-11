#include "sheetcomp.h"
#include "note.h"
#include <type_traits>



void sheetMeasure::addSpace(int space) {
  spacing.push_back(space);
}
    
void sheetMeasure::buildChordMap(vector<sheetNote>& vecNote) {
  // intermediate representation of chordmap (needs to be tick-sorted)
  set<pair<int, vector<sheetNote*>>, chordCmp> chordSet;
  
  for (auto& n : vecNote) {
    auto it = chordSet.find(make_pair(n.displayBegin, vector<sheetNote*>{}));
    if (it != chordSet.end()) {
      // add to chordSet
      auto vec = const_cast<vector<sheetNote*>*>(&(it->second));
      vec->push_back(&n);
    }
    else {
      pair<int, vector<sheetNote*>> newPos = make_pair(n.displayBegin, vector<sheetNote*>{&n});
      chordSet.insert(newPos);
    }
  }
  chords = vector<pair<int,vector<sheetNote*>>>(make_move_iterator(chordSet.begin()), make_move_iterator(chordSet.end()));
  //chords = vector<pair<int,vector<sheetNote*>>>(chordSet.begin(), chordSet.end());
}
