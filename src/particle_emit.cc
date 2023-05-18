#include "particle_emit.h"
#include "define.h"

void emitter::init(unsigned int idx) {
  constexpr int init_parts = 50; 

  part_vec.reserve(init_parts);

  for (int i = 0; i < init_parts; i++) {
    create_particle(idx);
  }
}

void emitter::update_part(double t_step) {
  for (auto& p : part_vec) {
    p.update(t_step);
  }
}

void emitter::update_data(const particleInfo& p_info) {
  data = p_info;
}

void emitter::render() {

}
    
void emitter::create_particle(unsigned int idx) {
  const note& n = ctr.getNotes()[idx];

  //part_vec.push_back(particle(n




}
