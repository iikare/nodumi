#include "particle_emit.h"
#include "define.h"
#include "wrap.h"
#include "lerp.h"

using std::remove_if;

void emitter::init(const pair<int, particleInfo>& p_info) {
  part_vec.reserve(max_parts);

  update_data(p_info.second);
  for (int i = 0; i < max_cycle_parts; i++) {
    create_particle();
  }
}

void emitter::update_part(double t_step, double zoom) {
  //logW(LL_WARN, "update dTime:" ,t_step);
  double update_ratio = 1.0;
  if (ctr.getPauseTime() > 0) {
    constexpr double p_int = 0.5;
    clampValue(clampValue((p_int-ctr.getPauseTime())/p_int, 0.0, 1.0)-0.4, 0.0, 1.0);
  }
  for (auto& p : part_vec) {
    p.update(t_step, zoom, update_ratio);
  }
  part_vec.erase(remove_if(part_vec.begin(), part_vec.end(), 
                 [&](const particle& p) { return p.is_end() || 0/*(1-p.ratio()) > update_ratio*/; }), part_vec.end());
}

void emitter::update_data(const particleInfo& p_info) {
  active = true;
  data = p_info;


  int add_part = 0;

  while (add_part < max_cycle_parts && part_vec.size() != max_parts) {
    create_particle();    
    add_part++;
  }

}

void emitter::render() {
  for (const auto& p : part_vec) {
    auto col = colorLERP(data.col, data.col_inv, p.ratio(), INT_ILINEAR); 
    //drawCircle(p.x, p.y, 2*p.ratio(), col, 120+125*p.ratio());
    drawPixel(p.x, p.y, col, 120+125*p.ratio());
  }
  //part_vec.erase(remove_if(part_vec.begin(), part_vec.end(), [](const particle& p) { return p.is_end(); }), part_vec.end());
}
    
void emitter::create_particle() {
  //const note& n = ctr.getNotes()[idx];

  //part_vec.push_back(particle(n


  constexpr float angle_start = 165.0;
  constexpr float angle_range = 30.0;

  const double t_life = 1.0 +2*ctr.getRandClamp();


  const double x = data.x + (data.w)*ctr.getRandClamp();
  const double y = data.y + (data.h)*ctr.getRandClamp();

  part_vec.emplace_back(x, y, t_life, ctr.getRandClamp()*angle_range+angle_start, 4);





}
