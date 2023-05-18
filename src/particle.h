#pragma once

#include <algorithm>
#include <vector>
#include <map>
#include "particle_inst.h"
#include "particle_emit.h"
#include "particle_info.h"
#include "color.h"

using std::vector;
using std::map;
using std::pair;

class particleController {
  public:

    void update(double zoom);
    void add_emitter(int index, const particleInfo& p_info);
    void end_emission();
    void process();
    void render();


  private:
    vector<pair<int, particleInfo>> current_emit;
    vector<pair<int, particleInfo>> current_emit_last;


    vector<int> emitter_idx;

    map<pair<int, particleInfo>, emitter, set_comp> emitter_map;
};
