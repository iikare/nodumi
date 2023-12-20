#include "build_target.h"

#include <cctype>
#include <limits>
#include <raylib.h>
#include "log.h"
#include "wrap.h"
#include "misc.h"
#include "buffer.h"

ACTION bufferController::process() {
  ACTION result = ACTION::NONE;
  if (isKeyPressed(KEY_ENTER)) {
    result = pending_action(true);
    if (result != ACTION::NONE) {
      return result;
    }
  }
  else {
    int key = GetCharPressed();
    if (isKeyPressed(KEY_BACKSPACE) && !empty()) {
      buf.pop_back();
    }
    while (key) {
      //logQ("f",static_cast<char>(key));
      if (key != ' ' || entry_in_progress()) {
        if (isKeyPressed(KEY_BACKSPACE) && !empty()) {
          buf.pop_back();
        }
        else {
          buf.push_back(key);

          result = pending_action();
          if (result != ACTION::NONE) {
            break;
          }
        }
      }
      key = GetCharPressed();
    }
  }
  return result;
}

ACTION bufferController::pending_action(bool apply_enter) {

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
    return ACTION::NAV_PREV_MEASURE;
  }
 
  if (apply_enter) {
    clear();
    if (sbuf == ":q") {
      return ACTION::CLOSE;
    }
    if (sbuf == ":qi") {
      return ACTION::CLOSE_IMAGE;
    }
    if (sbuf == ":qa") {
      return ACTION::EXIT;
    }
    if (sbuf == ":o") {
      return ACTION::OPEN;
    }
    if (sbuf == ":oi") {
      return ACTION::OPEN_IMAGE;
    }
    if (sbuf == ":p") {
      return ACTION::PREFERENCES;
    }
    if (sbuf == ":s") {
      return ACTION::SHEET;
    }
    if (sbuf == ":i") {
      return ACTION::PREFERENCES;
    }
  }

  if (sbuf.length() > 1 && (sbuf.back() == 'G' || sbuf.back() == 'w' || sbuf.back() == 'b')) {
    bool all_num = true;
    for (unsigned int i = 0; i < sbuf.size() - 1; ++i) {
      if (!isdigit(sbuf[i])) {
        all_num = false;
        break;
      }
    }
    clear();
    if (all_num) {
      if (sbuf.size() < 11) {
        set_pending(stoi(sbuf.substr(0,sbuf.size()-1)));
      }
      else {
        set_pending(std::numeric_limits<int>::max());
      }
      switch (sbuf.back()) {
        case 'G':
          return ACTION::NAV_SET_MEASURE; 
        case 'w':
          return ACTION::NAV_NEXT_MEASURE;
        case 'b':
          return ACTION::NAV_PREV_MEASURE;
      }
    }
  }


  return ACTION::NONE;
}
