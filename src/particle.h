#pragma once

#include <algorithm>
#include <vector>

using std::vector;
using std::pair;

class particleController {
  public:

    void update();
    void add_emitter(int index);
    void end_emission();
    void process();


  private:
    vector<int> current_emit;
    vector<int> current_emit_last;


    vector<int> emitter_idx;

};
