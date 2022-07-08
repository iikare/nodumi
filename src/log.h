#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using std::cerr; 
using std::endl;
using std::ostringstream;
using std::string;
using std::vector;
using std::pair;
using std::make_pair;
using std::fixed;
using std::setprecision;

#define logW(a, ...) logOutput(a, __FILE__, __LINE__, __VA_ARGS__)

#ifndef NO_DEBUG
  #define logQ(...) logOutput(LL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
  #define logC(a,b) logOutput(LL_CRIT, __FILE__, __LINE__, a, "v.", b)
#else
  #define logQ(...) ;
  #define logC(a,b) ;
#endif
enum logLevel {
  LL_INFO,
  LL_WARN,
  LL_CRIT,
  LL_DEBUG
};

void logProcess(const logLevel& level, string&lvmsg, string& file);

template<typename U, typename V>
U& logRecursive(U& stream, const V& arg1) {
   stream << " ";
   return (stream << arg1);
}

template<typename U, typename V, typename... W>
U& logRecursive(U&  stream, const V& arg1, const W&... args) {
   stream << " ";
   return logRecursive((stream << arg1), args...);
}

template<typename V, typename... W>
void logOutput(logLevel level, string file, int line, const V& arg1, const W&... args) {
  
  #ifdef NO_DEBUG
    if (level == LL_DEBUG) { return; }
  #endif
  
  ostringstream out;
  string lvmsg = "";

  logProcess(level, lvmsg, file);
  
  out << lvmsg;
  logRecursive((out << arg1), args...);
  out << " → " << file << ":" << line << endl;

  cerr << out.str();
}

template<typename V>
void logOutput(logLevel level, string file, int line, const V& arg1) {
 
  #ifdef NO_DEBUG
    if (level == LL_DEBUG) { return; }
  #endif 

  ostringstream out;
  string lvmsg = "";

  logProcess(level, lvmsg, file);

  out << lvmsg << arg1 << " → " << file << ":" << line << endl;

  cerr << out.str();
}

template<typename U, typename V>
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

template<typename T>
string formatVector(const vector<T>& vec) {
  
  #ifdef NO_DEBUG
    return "";
  #endif
  
  string s;
  
  for (typename vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {

    ostringstream ss;
    ss << fixed << setprecision(2);
    ss << *it;
    s += ss.str();
    s += " ";
  }
  
  if (s.size() >= 1) {
    s.erase(s.size()-1);
  }

  return s;
}

template<typename U, typename V>
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
    s.erase(s.size()-1);
  }
  
  return s;
}

