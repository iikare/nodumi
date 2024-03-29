#pragma once

#include <chrono>
#include <string>
#include <type_traits>
#include <vector>

#include "box.h"
#include "build_target.h"
#include "cie2k.h"
#include "color.h"
#include "note.h"

using std::is_enum;
using std::is_same;
using std::string;
using std::vector;

double getDistance(int x1, int y1, int x2, int y2);

template <class U, class... V>
bool any_of(U target, V... args) {
  return (... || (target == args));
}

template <class U, class... V>
bool all_of(U target, V... args) {
  return (... && (target == args));
}

template <class T>
T clampValue(T val, T lo, T hi) {
  return val > hi ? hi : (val < lo ? lo : val);
}

template <class T>
bool pointInBox(const T& pt, const rect& box) {
  if constexpr (is_same<T, Vector2>::value || is_same<T, point>::value) {
    return static_cast<int>(pt.x) >= box.x && static_cast<int>(pt.x) < box.x + box.width &&
           static_cast<int>(pt.y) >= box.y && static_cast<int>(pt.y) < box.y + box.height;
  }

  // logW(LL_WARN, "call to", __func__, "using invalid type", typeid(T).name());
  return false;
}
rect pointToRect(const point& a, const point& b);
point getMousePosition();

string colorToHex(const colorRGB& col);

string getNoteInfo(int noteTrack, int notePos, bool isFlat);

string getSongPercent(double pos);
string getSongTime(double pos);
string toMinutes(double seconds);

vector<string>& formatPortName(vector<string>& ports);

bool isValidPath(const string& path, int pathTypes...);
bool isValidExtension(const string& ext, int pathType);
bool isValidExtension(const string& ext, int pathType, int pathTypes...);
string getExtension(const string& path, bool len4 = false);
string getDirectory(const string& path);

string toHex(int dec);

colorRGB maximizeDeltaE(const colorRGB& ref);

template <class T>
vector<int> convertEnum(const vector<T>& ev) {
  static_assert(is_enum<T>::value, "cannot convert non-enum type to int vector");

  vector<int> result;
  result.resize(ev.size());
  for (unsigned int i = 0; i < ev.size(); ++i) {
    result[i] = static_cast<int>(ev[i]);
  }
  return result;
}

template <class T>
void debug_time(const T& start, string msg = "") {
  T end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsedTime = end - start;
  msg = (msg == "") ? "seconds" : "seconds (" + msg + ")";
  logW(LL_INFO, "elapsed time:", elapsedTime.count(), msg);
}
