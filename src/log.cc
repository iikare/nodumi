#include <iostream>
#include <iomanip>
#include <string>
#include "log.h"

using std::cerr;
using std::endl;
using std::string;
using std::to_string;

void logOutput (int level, string msg, double value, string file, int line) {
  if (value - static_cast<int>(value) < 0.0001) {
    logOutput(level, msg + ": " + to_string(static_cast<int>(value)), file, line);
  }
  else {
    logOutput(level, msg + ": " + to_string(value), file, line);
  }
}

void logOutput(int level, double msg, string file, int line) {
  if (msg - static_cast<int>(msg) < 0.00000001) {
    logOutput(level, to_string(static_cast<int>(msg)), file, line);
  }
  else {
    logOutput(level, to_string(msg), file, line);
  }
}

void logOutput(int level, string msg, string file, int line) {
  string lvmsg = "";
  if (level == LL_INFO) {
    lvmsg = "INFO: ";
  }
  else if (level == LL_WARN) {
    lvmsg = "WARN: ";
  }
  else if (level == LL_CRIT) {
    lvmsg = "CRITICAL ERROR: ";
  }
  else {
    lvmsg = "undefined error level: ";
  }

  for (unsigned int i = 0; i < file.length(); i++) {
    if (file[file.length() - i] == '/') {
      file = file.substr(file.length() - i + 1, file.length());
    }
  }

  cerr << lvmsg << msg << " → " << file << ":" << line << endl;
}
