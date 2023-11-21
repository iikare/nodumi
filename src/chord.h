#pragma once

#include <vector>
#include "note.h" 

using std::vector;

class chordData {
  public:
    chordData(unsigned int initial_idx) {
      member = {initial_idx};
    }
    
    void sort_asc(vector<note>* n_vec);
    void add_member(unsigned int idx) { member.push_back(idx); }
    
    int latest_end(vector<note>* n_vec) const;
    unsigned int size() const { return member.size(); }
    const vector<unsigned int>& data() { return member; }

  private:
    vector<unsigned int> member;
};
