#include "option.h"
#include "enum.h"
#include "define.h"

optionController::optionController() {
  opts.resize(static_cast<int>(optionType::OPTION_NONE), 0);
  opts[static_cast<int>(optionType::OPTION_TRACK_DIVISION)] = true;
}

void optionController::invert(optionType opt) {
  
  switch (opt) {
    case optionType::OPTION_TRACK_DIVISION:
      if (!get(opt)) {
        getColorScheme(2, ctr.setTrackOn, ctr.setTrackOff);
      }
      break;
    default:
      break;
  }

  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

bool optionController::get(optionType opt) {
  return opts[static_cast<int>(opt)];
}
