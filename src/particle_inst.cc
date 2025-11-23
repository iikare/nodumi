#include "particle_inst.h"

#include "define.h"
void particle::update(double t_step, double zoom, double ratio) {
  r_life -= t_step * rand() / float(RAND_MAX);
  vel = 2 * r_life / t_life * (rand() / float(RAND_MAX));

  auto x_off = ratio * zoom * 500 * vel * t_step * cos(angle * M_PI / 180.0);
  x += (x_off > 0) ? 0.2 * x_off : x_off;
  y += ratio * zoom * 500 * vel * t_step * sin(angle * M_PI / 180.0);

  // logQ(100*vel*t_step*sin(angle*M_PI/180.0));
}
