#include "log.h"
#include "text.h"

using std::make_pair;

string textController::getString(const string& str_id) const {
  auto lang_it = labelMap.find(lang);
  auto it = lang_it->second.find(str_id);

  //logQ("getString() called with ID:", str_id);

  if (it == lang_it->second.end()) {
    logW(LL_WARN, "invalid text label:", str_id);
  }
  else {
    return it->second;
  }
  return {};
}

void textController::setLanguage(langType lang) {
  switch (lang) {
    using enum langType;
    default:
      logW(LL_WARN, "invalid language option -", static_cast<int>(lang));
  }
}

void textController::init() {
  lang = langType::LANG_EN;
  labelMap.insert(make_pair(lang, LABEL_LANG_EN));
}
