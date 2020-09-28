#pragma once

class note {
  public:
    note() : track(0), duration(0), x(0), y(0), velocity(0), time(0), isOn(0) {}
    
    int track;
    double duration;
    double x;
    int y;
    int velocity;
    double time;
    bool isOn;

};
