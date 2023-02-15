#include "log.h"
#include "text.h"

string textController::getString(const string& str_id) const {
  return "not impl.";
}

void textController::setLanguage(langType lang) {
  switch (lang) {

    default:
      logW(LL_WARN, "invalid language option -", static_cast<int>(lang));

  }
}
