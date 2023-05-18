#pragma once

#include <vector>
#include "particle_inst.h"
#include "particle_info.h"

using std::vector;
using std::pair;

class emitter {
  
  public:

    void init(const pair<int, particleInfo>& p_info);
    void update_part(double t_step);
    void update_data(const particleInfo& p_info);
    void render();


  private:

    void create_particle();

    particleInfo data;

    vector<particle> part_vec;
};
