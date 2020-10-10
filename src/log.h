#pragma once

#include <iostream>
#include <string>

#define logII(a, b) logOutput(a, b, __FILE__, __LINE__)
#define log3(a, b, c) logOutput(a, b, c, __FILE__, __LINE__)

using std::cerr;
using std::endl;
using std::string;

enum logLevel {
  LL_INFO,
  LL_WARN,
  LL_CRIT
};

void logOutput(int level, double msg, const string& file, int line);
void logOutput (int level, const string& msg, string file, int line);
void logOutput (int level, const string& msg, double value, const string& file, int line);
