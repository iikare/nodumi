
#ifdef __WIN32
  #include<windows.h>
#endif

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

void writeColor(const logLevel& level, string&lvmsg, ostringstream& out) {
  const char* csel[7] = {"\033[0;34m", "\033[0;32m", // blue   , green
                      "\033[0;36m", "\033[0;31m", // cyan   , red
                      "\033[0;35m", "\033[0;33m", // magenta, yellow
                      "\033[0m"};                 // default
  int cpos = 7;
  switch (level) {
    case LL_DEBUG:
      cpos = 1;
      break;
    case LL_INFO:
      cpos = 3;
      break;
    case LL_WARN:
      cpos = 6;
      break;
    case LL_CRIT:
      cpos = 4;
      break;
  }

  #ifdef __WIN32
    HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
    SetConsoleTextAttribute(hConsole, cpos);
    out << lvmsg;
    SetConsoleTextAttribute(hConsole, 7);
  #else
    out << csel[cpos-1] << lvmsg << "\033[0m";
  #endif
}
