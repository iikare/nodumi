#pragma once

#include <math.h>

#include <cstdlib>

#include "build_target.h"
#include "log.h"

class particle {
 public:
  particle() = delete;
  particle(double n_x, double n_y, double n_t_life, double n_angle,
           double n_vel) {
    x = n_x;
    y = n_y;
    t_life = n_t_life;
    r_life = n_t_life;
    angle = n_angle;
    vel = n_vel;
  }

  particle(const particle& n) {
    x = n.x;
    y = n.y;
    t_life = n.t_life;
    angle = n.angle;
    vel = n.vel;
  }

  particle& operator=(const particle& n) {
    x = n.x;
    y = n.y;
    t_life = n.t_life;
    angle = n.angle;
    vel = 2;  // n.vel;

    return *this;
  }

  void update(double t_step, double zoom, double ratio);

  double ratio() const { return r_life / t_life; }
  bool is_end() const { return r_life < 0; }

  double x;
  double y;

 private:
  double r_life;
  double t_life;
  double angle;
  double vel;
};
