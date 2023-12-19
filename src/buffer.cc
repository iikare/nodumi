#include "build_target.h"

#include <cctype>
#include <raylib.h>
#include "log.h"
#include "wrap.h"
#include "buffer.h"

ACTION bufferController::process() {
  int key = GetCharPressed();
  if (isKeyPressed(KEY_BACKSPACE) && !empty()) {
    buf.pop_back();
  }
  while (key) {
    //logQ("f",static_cast<char>(key));
    if (key != ' ') {
      if (isKeyPressed(KEY_BACKSPACE) && !empty()) {
        buf.pop_back();
      }
      else {
        buf.push_back(key);

        ACTION result = pending_action();
        if (result != ACTION::NONE) {
          return result;
        }
      }
    }
    key = GetCharPressed();
  }
  return ACTION::NONE;
}

ACTION bufferController::pending_action() {

  string sbuf = read();

  if (sbuf == "gg" || sbuf == "0") {
    clear();
    return ACTION::NAV_HOME;
  }
  if (sbuf == "G" || sbuf == "$") { 
    clear();
    return ACTION::NAV_END;
  }
  if (sbuf == "w") {
    clear();
    return ACTION::NAV_NEXT_MEASURE;
  }
  if (sbuf == "b") {
    clear();
    return ACTION::NAV_PREV_MEASURE;;
  }

  if (sbuf.length() > 1 && sbuf.back() == 'G') {
    bool all_num = true;
    for (unsigned int i = 0; i < sbuf.size() - 1; ++i) {
      if (!isdigit(sbuf[i])) {
        all_num = false;
        break;
      }
    }
    clear();
    if (all_num) {
      set_pending(stoi(sbuf.substr(0,sbuf.size()-1)));
      return ACTION::NAV_SET_MEASURE; 
    }
  }


  return ACTION::NONE;
}
