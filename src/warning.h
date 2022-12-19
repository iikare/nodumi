#pragma once

#include "build_target.h"

#include <string>

using std::string;

class warningController {
  public:
    
    void init();

    void render();

  private:
  
  #if defined(TARGET_WIN)
    bool win_warning = true;
    string win_warn_msg = "WARNING: windows version is experimental";
    static constexpr int win_warn_fsize = 24;
    static constexpr int win_warn_spacing = 36;
    static constexpr int win_warn_offset = 24;
    Vector2 win_warn_msg_size;
  #endif

};
