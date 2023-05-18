#pragma once

#include "build_target.h"
#include <cstdlib>
#include <math.h>

class particle {
  public:
    
    particle() = delete;
    particle(double n_x, double n_y, double n_t_life, double n_angle, /*double n_vel,*/ int k_idx) {

      x = n_x;
      y = n_y;
      t_life = n_t_life;
      r_life = n_t_life;
      angle = n_angle;
      vel = 2;//n_vel;
      idx = k_idx;
    }

    //particle(const particle& n) {
      //x = n.x;
      //y = n.y;
      //t_life = n.t_life;
      //angle = n.angle;
      //vel = n.vel;
    //}

    particle& operator ==(const particle& n) {
      x = n.x;
      y = n.y;
      t_life = n.t_life;
      angle = n.angle;
      vel = 2;//n.vel;

      idx = n.idx;

      return *this;
    }


    double ratio() { return r_life/t_life; }

    void update(double t_step) {
      r_life -= t_step* rand()/float(RAND_MAX);
      vel = 2 * r_life/t_life * (rand()/float(RAND_MAX));
      x += 0.1*vel*t_step*cos(angle*M_PI/180.0);
      y += 0.1*vel*t_step*sin(angle*M_PI/180.0);



      
    }


    bool is_end() { return r_life < 0; }
    
    double x;
    double y;

    int idx;

  private:
    double r_life;
    double t_life;
    double angle;
    double vel;
};

