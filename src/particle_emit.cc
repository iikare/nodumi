#include "particle_emit.h"
#include "define.h"
#include "wrap.h"

void emitter::init(const pair<int, particleInfo>& p_info) {
  constexpr int init_parts = 50; 

  part_vec.reserve(init_parts);

  update_data(p_info.second);

  for (int i = 0; i < init_parts; i++) {
    create_particle();
  }
}

void emitter::update_part(double t_step) {

  logW(LL_WARN, "update dTime:" ,t_step);
  for (auto& p : part_vec) {
    p.update(t_step);
  }
}

void emitter::update_data(const particleInfo& p_info) {
  data = p_info;
}

void emitter::render() {

  for (const auto& p : part_vec) {

    drawRing({static_cast<float>(p.x), static_cast<float>(p.y)}, 0, 5*p.ratio(), data.col);

  }

}
    
void emitter::create_particle() {
  //const note& n = ctr.getNotes()[idx];

  //part_vec.push_back(particle(n


  constexpr float angle_start = 140.0;
  constexpr float angle_range = 50.0;

  constexpr double t_life = 4.0;



  particle p = particle(data.x, data.y, t_life, ctr.getRandClamp()*angle_range+angle_start, 0);
  

  part_vec.emplace_back(data.x, data.y, t_life, ctr.getRandClamp()*angle_range+angle_start, 0);





}
