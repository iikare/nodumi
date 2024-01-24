#include "buffer.h"

#include <cctype>
#include <limits>

#include "build_target.h"
#include "log.h"
#include "misc.h"
#include "wrap.h"

ACTION
bufferController::process() {
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
      // logQ("f",static_cast<char>(key));
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

ACTION
bufferController::pending_action(bool apply_enter) {
  string sbuf = read();

  if (any_of(sbuf, "gg", "0")) {
    clear();
    return ACTION::NAV_HOME;
  }
  if (any_of(sbuf, "G", "$")) {
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
    if (sbuf == ":f") {
      return ACTION::FILE_INFO;
    }
    if (sbuf == ":i") {
      return ACTION::INFO;
    }
    if (sbuf == ":l") {
      return ACTION::LIVEPLAY;
    }
    if (sbuf.substr(0, 2) == ":d") {
      if (sbuf.size() > 2 && all_num(sbuf.substr(2))) {
        string disp_num = sbuf.substr(2);
        if (disp_num.size() < 2) {
          int disp_val = stoi(disp_num);
          if (disp_val != 0) {
            set_pending(min(DISPLAY_NONE - 1, disp_val - 1));
            return ACTION::CHANGE_MODE;
          }
        }
      }
    }
  }

  if (sbuf.length() > 1 && any_of(sbuf.back(), 'G', 'w', 'b')) {
    clear();
    if (all_num(sbuf.substr(0, sbuf.size() - 2))) {
      if (sbuf.size() < 11) {
        set_pending(stoi(sbuf.substr(0, sbuf.size() - 1)));
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

bool bufferController::all_num(const string& ref_str) const {
  for (unsigned int i = 0; i < ref_str.size(); ++i) {
    if (!isdigit(ref_str[i])) {
      return false;
    }
  }
  return true;
}
