#include "particle_inst.h"

void particle::update(double t_step) {
  r_life -= t_step* rand()/float(RAND_MAX);
  vel = 2 * r_life/t_life * (rand()/float(RAND_MAX));


  x += 100*vel*t_step*cos(angle*M_PI/180.0);
  y += 100*vel*t_step*sin(angle*M_PI/180.0);
  
  logQ(100*vel*t_step*sin(angle*M_PI/180.0));





  
}
