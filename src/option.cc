#include "option.h"
#include "data.h"
#include "enum.h"
#include "define.h"

optionController::optionController() {
  opts.resize(static_cast<int>(optionType::OPTION_NONE), 0);
  opts[static_cast<int>(optionType::OPTION_TRACK_DIVISION)] = true;
  opts[static_cast<int>(optionType::OPTION_SET_HAND_RANGE)] = false;
  opts[static_cast<int>(optionType::OPTION_HAND_RANGE)] = MAX_HAND_RANGE;
  opts[static_cast<int>(optionType::OPTION_SET_DARKEN_IMAGE)] = false;
  opts[static_cast<int>(optionType::OPTION_CIE_FUNCTION)] = static_cast<int>(CIE::C_00);
  opts[static_cast<int>(optionType::OPTION_SET_CIE_FUNCTION)] = false;
  opts[static_cast<int>(optionType::OPTION_DARKEN_IMAGE)] = 0;
  opts[static_cast<int>(optionType::OPTION_DYNAMIC_LABEL)] = 1;
}

void optionController::invert(optionType opt) {
  if (invalid(opt)) { return; }

  switch (opt) {
    case optionType::OPTION_TRACK_DIVISION:
      if (!get(opt)) {
        getColorScheme(2, ctr.setTrackOn, ctr.setTrackOff);
      }
      break;
    case optionType::OPTION_DYNAMIC_LABEL:
      ctr.optimizeBGColor(true);
      break;
    case optionType::OPTION_SET_HAND_RANGE:
      break;
    case optionType::OPTION_SET_DARKEN_IMAGE:
      break;
    case optionType::OPTION_SET_CIE_FUNCTION:
      break;
    default:
      logW(LL_WARN, "cannot invert option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

bool optionController::invalid(optionType opt) {
  switch(opt) {
    case optionType::OPTION_SET_HAND_RANGE:
      [[fallthrough]];
    case optionType::OPTION_HAND_RANGE:
      return !get(optionType::OPTION_TRACK_DIVISION);
    case optionType::OPTION_SET_DARKEN_IMAGE:
      [[fallthrough]];
    case optionType::OPTION_DARKEN_IMAGE:
      return !ctr.image.exists();
    default:
      return false;
  }
}

void optionController::set(optionType opt, int value) {

  if (invalid(opt)) { return; }
  
  switch (opt) {
    case optionType::OPTION_HAND_RANGE:
      [[fallthrough]];
    case optionType::OPTION_DARKEN_IMAGE:
      [[fallthrough]];
    case optionType::OPTION_CIE_FUNCTION:
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
