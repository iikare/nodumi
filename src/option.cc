#include "option.h"
#include "data.h"
#include "enum.h"
#include "define.h"

optionController::optionController() {
  opts.resize(static_cast<int>(optionType::OPTION_NONE), 0);
  opts[static_cast<int>(optionType::OPTION_TRACK_DIVISION)] = true;
  opts[static_cast<int>(optionType::OPTION_SET_HAND_RANGE)] = false;
  opts[static_cast<int>(optionType::OPTION_HAND_RANGE)] = MAX_HAND_RANGE;
}

void optionController::invert(optionType opt) {
  switch (opt) {
    case optionType::OPTION_TRACK_DIVISION:
      if (!get(opt)) {
        getColorScheme(2, ctr.setTrackOn, ctr.setTrackOff);
      }
      break;
    case optionType::OPTION_SET_HAND_RANGE:
      break;
    default:
      logW(LL_WARN, "cannot invert option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

void optionController::set(optionType opt, int value) {
  switch (opt) {
    case optionType::OPTION_HAND_RANGE:
      break;
    default:
      logW(LL_WARN, "cannot modify option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = value;
}

int optionController::get(optionType opt) {
  return opts[static_cast<int>(opt)];
}
