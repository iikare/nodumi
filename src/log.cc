#include <string>
#include "log.h"

using std::string;

void logProcess(const logLevel& level, string&lvmsg, string& file) {
  if (level == LL_INFO) {
    lvmsg = "INFO: ";
  }
  else if (level == LL_WARN) {
    lvmsg = "WARN: ";
  }
  else if (level == LL_CRIT) {
    lvmsg = "CRITICAL ERROR: ";
  }
  else if (level == LL_DEBUG) {
    lvmsg = "DEBUG: ";
  }
  else {
    lvmsg = "undefined error level: ";
  }

  for (unsigned int i = 0; i < file.length(); i++) {
    if (file[file.length() - i] == '/') {
      file = file.substr(file.length() - i + 1, file.length());
    }
  }
}
