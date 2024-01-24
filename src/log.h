#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef __WIN32
  #define LOGQ_SEP " -> "
#else
  #define LOGQ_SEP " ➜ "
#endif

using std::cerr;
using std::endl;
using std::false_type;
using std::fixed;
using std::is_enum;
using std::ostringstream;
using std::pair;
using std::setprecision;
using std::string;
using std::true_type;
using std::vector;

#define logW(a, ...) logOutput(a, __FILE__, __LINE__, __VA_ARGS__)

#ifndef NO_DEBUG
  #define logQ(...) logOutput(LL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
  #define logC(a, b) logOutput(LL_CRIT, __FILE__, __LINE__, a, "v.", b)
  #define logE() cerr << endl
#else
  #define logQ(...) ;
  #define logC(a, b) ;
  #define logE() cerr << endl
#endif
enum logLevel { LL_INFO, LL_WARN, LL_CRIT, LL_DEBUG };

inline void logProcess(const logLevel& level, string& lvmsg, string& file) {
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

#ifdef __WIN32
// colored output not supported
inline void writeColor(string& lvmsg, ostringstream& out) {
#else
inline void writeColor(const logLevel& level, string& lvmsg,
                       ostringstream& out) {
#endif

#ifdef __WIN32
  // HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
  // SetConsoleTextAttribute(hConsole, cpos);
  out << lvmsg;
  // SetConsoleTextAttribute(hConsole, 7);
#else
  const char* csel[7] = {"\033[0;34m", "\033[0;32m",  // blue   , green
                         "\033[0;36m", "\033[0;31m",  // cyan   , red
                         "\033[0;35m", "\033[0;33m",  // magenta, yellow
                         "\033[0m"};                  // default

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
  out << csel[cpos - 1] << lvmsg << "\033[0m";
#endif
}

template <typename T>
struct is_vector {
  static constexpr bool value = false;
};

template <typename T>
struct is_vector<std::vector<T>> {
  static constexpr bool value = true;
};

template <typename T>
struct is_pair {
  static constexpr bool value = false;
};

template <typename U, typename V>
struct is_pair<pair<U, V>> {
  static constexpr bool value = true;
};

template <typename U, typename V>
string formatPair(const pair<U, V>& typePair) {
#ifdef NO_DEBUG
  return "";
#endif

  string s;

  ostringstream ss;
  ss << fixed << setprecision(2);
  ss << (typePair.first) << " " << (typePair.second);
  s += ss.str();

  return s;
}

template <typename T>
string formatVector(const vector<T>& vec) {
#ifdef NO_DEBUG
  return "";
#endif

  string s;

  for (typename vector<T>::const_iterator it = vec.begin(); it != vec.end();
       ++it) {
    ostringstream ss;
    ss << fixed << setprecision(2);
    ss << *it;
    s += ss.str();
    s += " ";
  }

  if (s.size() >= 1) {
    s.erase(s.size() - 1);
  }

  return s;
}

template <typename U, typename V>
string formatVector(const vector<pair<U, V>>& vec) {
#ifdef NO_DEBUG
  return "";
#endif

  string s;

  for (auto it = vec.begin(); it != vec.end(); ++it) {
    s += formatPair(*it);
    s += " ";
  }

  if (s.size() >= 1) {
    s.erase(s.size() - 1);
  }

  return s;
}

template <typename U, typename V>
U& logRecursive(U& stream, const V& arg1) {
  stream << " ";
  if constexpr (is_vector<V>::value) {
    return (stream << formatVector(arg1));
  }
  else if constexpr (is_pair<V>::value) {
    return (stream << formatPair(arg1));
  }
  else if constexpr (is_enum<V>::value) {
    return (stream << static_cast<int>(arg1));
  }
  else {
    return (stream << arg1);
  }
}

template <typename U, typename V, typename... W>
U& logRecursive(U& stream, const V& arg1, const W&... args) {
  stream << " ";
  if constexpr (is_vector<V>::value) {
    return logRecursive((stream << formatVector(arg1)), args...);
  }
  else if constexpr (is_pair<V>::value) {
    return logRecursive((stream << formatPair(arg1)), args...);
  }
  else if constexpr (is_enum<V>::value) {
    return logRecursive((stream << static_cast<int>(arg1)), args...);
  }
  else {
    return logRecursive((stream << arg1), args...);
  }
}

template <typename V, typename... W>
void logOutput(logLevel level, string file, int line, const V& arg1,
               const W&... args) {
#ifdef NO_DEBUG
  if (level == LL_DEBUG) {
    return;
  }
#endif

  ostringstream out;
  string lvmsg = "";

  logProcess(level, lvmsg, file);
#ifdef __WIN32
  writeColor(lvmsg, out);
#else
  writeColor(level, lvmsg, out);
#endif

  if constexpr (is_vector<V>::value) {
    logRecursive((out << formatVector(arg1)), args...);
  }
  else if constexpr (is_pair<V>::value) {
    logRecursive((out << formatPair(arg1)), args...);
  }
  else if constexpr (is_enum<V>::value) {
    logRecursive((out << static_cast<int>(arg1)), args...);
  }
  else {
    logRecursive((out << arg1), args...);
  }

  out << LOGQ_SEP << file << ":" << line << endl;

  cerr << out.str();
}

template <typename V>
void logOutput(logLevel level, string file, int line, const V& arg1) {
#ifdef NO_DEBUG
  if (level == LL_DEBUG) {
    return;
  }
#endif

  ostringstream out;
  string lvmsg = "";

  logProcess(level, lvmsg, file);
#ifdef __WIN32
  writeColor(lvmsg, out);
#else
  writeColor(level, lvmsg, out);
#endif

  if constexpr (is_vector<V>::value) {
    out << formatVector(arg1);
  }
  else if constexpr (is_pair<V>::value) {
    out << formatPair(arg1);
  }
  else if constexpr (is_enum<V>::value) {
    out << static_cast<int>(arg1);
  }
  else {
    out << arg1;
  }

  out << LOGQ_SEP << file << ":" << line << endl;

  cerr << out.str();
}
