#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "data_str.h"
#include "enum.h"

using std::decay_t;
using std::is_same;
using std::map;
using std::string;
using std::unordered_map;

class textController {
 public:
  void init();

  string getString(const string& str_id) const;

  // template<typename V, typename... W>

  template <typename V, typename... W>
  vector<string> getStringSet(const V& str_id, const W&... str_ids) const {
    static_assert(
        is_same<decay_t<V>, string>::value || is_same<decay_t<V>, char*>::value,
        "bad type passed in getStringSet()");
    vector<string> str_set;

    getStringSet(str_set, str_id, str_ids...);

    return str_set;
  }

  template <typename V, typename... W>
  void getStringSet(vector<string>& str_set, const V& str_id,
                    const W&... str_ids) const {
    static_assert(
        is_same<decay_t<V>, string>::value || is_same<decay_t<V>, char*>::value,
        "bad type passed in getStringSet()");
    str_set.push_back(getString(str_id));
    getStringSet(str_set, str_ids...);
  }

  template <typename V>
  void getStringSet(vector<string>& str_set, const V& str_id) const {
    static_assert(
        is_same<decay_t<V>, string>::value || is_same<decay_t<V>, char*>::value,
        "bad type passed in getStringSet()");
    str_set.push_back(getString(str_id));
  }

  // vector<string> getStringSet(const string& str_ids...);

  void setLanguage(langType lang);

  langType getLanguage() const { return lang; }

 private:
  map<langType, unordered_map<string, string>> labelMap;

  langType lang;
};
