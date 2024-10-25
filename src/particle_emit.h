#pragma once

#include <vector>

#include "particle_info.h"
#include "particle_inst.h"

using std::pair;
using std::vector;

class emitter {
 public:
  void init(const pair<int, particleInfo>& p_info);
  void update_part(double t_step, double zoom);
  void update_data(const particleInfo& p_info);
  void render();

  bool active = false;

 private:
  unsigned int max_parts = 127;
  static constexpr unsigned int max_cycle_parts = 1;
  static constexpr double spp_const = 1.50;
  static constexpr double init_ratio = 0.25;

  void create_particle();

  particleInfo data;

  vector<particle> part_vec;
};
