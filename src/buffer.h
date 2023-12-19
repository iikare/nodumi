#pragma once

#include "build_target.h"

#include <vector>
#include <string>
#include "enum.h"

using std::vector;
using std::string;

class bufferController {
  public:


    bool empty() const { return !buf.size(); }
    void clear() { buf.clear(); }
    string read() { return string(buf.begin(), buf.end()); }


    ACTION process();
    int get_pending() const { return p_val; }
    


  private:
    ACTION pending_action();
    void set_pending(int val) { p_val = val; }

    int p_val;
    vector<char> buf;


};
