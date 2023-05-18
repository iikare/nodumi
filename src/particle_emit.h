#pragma once

#include <vector>
#include "particle_inst.h"
#include "particle_info.h"

using std::vector;

class emitter {
  
  public:

    void init(unsigned int idx);
    void update_part(double t_step);
    void update_data(const particleInfo& p_info);
    void render();


  private:

    void create_particle(unsigned int idx);

    particleInfo data;

    vector<particle> part_vec;
};
