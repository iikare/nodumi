#pragma once

#include <vector>
#include "particle_inst.h"
#include "particle_info.h"

using std::vector;
using std::pair;

class emitter {
  
  public:

    void init(const pair<int, particleInfo>& p_info);
    void update_part(double t_step, double zoom);
    void update_data(const particleInfo& p_info);
    void render();

    bool active = false;

  private:

    static constexpr int max_parts = 127; 
    static constexpr int max_cycle_parts = 2; 
    
    void create_particle();

    particleInfo data;

    vector<particle> part_vec;
};
