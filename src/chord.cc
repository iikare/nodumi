#include "chord.h"

#include <algorithm>

#include "log.h"

using std::sort;

void chordData::sort_asc(vector<note>* n_vec) {
  if (member.size() == 1) {
    return;
  }

  // logE();
  // logQ("START");
  // for (auto i : member) {
  // logQ( n_vec->at(i).y);
  //}
  // logE();

  auto y_comp = [=](const auto l, const auto r) {
    return n_vec->at(l).y < n_vec->at(r).y;
  };

  sort(member.begin(), member.end(), y_comp);
}

int chordData::latest_end(vector<note>* n_vec) const {
  auto end_comp = [=](const auto l, const auto r) {
    return n_vec->at(l).duration < n_vec->at(r).duration;
  };

  return *std::max_element(std::begin(member), std::end(member), end_comp);
}
