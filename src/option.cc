#include "option.h"
#include "cie2k.h"
#include "data.h"
#include "enum.h"
#include "define.h"

optionController::optionController() {
  opts.resize(static_cast<int>(optionType::OPTION_NONE), 0);
  opts[static_cast<int>(optionType::OPTION_TRACK_DIVISION)] = true;
  opts[static_cast<int>(optionType::OPTION_SET_HAND_RANGE)] = false;
  opts[static_cast<int>(optionType::OPTION_HAND_RANGE)] = MAX_HAND_RANGE;
  opts[static_cast<int>(optionType::OPTION_SET_DARKEN_IMAGE)] = false;
  opts[static_cast<int>(optionType::OPTION_CIE_FUNCTION)] = static_cast<int>(cie2k::TYPE::CIE_00);
  opts[static_cast<int>(optionType::OPTION_SET_CIE_FUNCTION)] = false;
  opts[static_cast<int>(optionType::OPTION_DARKEN_IMAGE)] = 0;
  opts[static_cast<int>(optionType::OPTION_DYNAMIC_LABEL)] = true;
  opts[static_cast<int>(optionType::OPTION_PARTICLE)] = false;
  opts[static_cast<int>(optionType::OPTION_SCALE_VELOCITY)] = false;
  opts[static_cast<int>(optionType::OPTION_SHADOW)] = false;
  opts[static_cast<int>(optionType::OPTION_SHADOW_DISTANCE)] = 8;
}

void optionController::invert(optionType opt) {
  if (invalid(opt)) { return; }

  switch (opt) {
    using enum optionType;
    case OPTION_TRACK_DIVISION:
      if (ctr.getLiveState() && !get(opt)) {
        getColorScheme(2, ctr.setTrackOn, ctr.setTrackOff);
      }
      break;
    case OPTION_DYNAMIC_LABEL:
      ctr.optimizeBGColor(true);
      break;
    case OPTION_SET_HAND_RANGE:
      break;
    case OPTION_SET_DARKEN_IMAGE:
      break;
    case OPTION_SET_CIE_FUNCTION:
      break;
    case OPTION_PARTICLE:
      break;
    case OPTION_SCALE_VELOCITY:
      break;
    case OPTION_SHADOW:
      break;
    default:
      logW(LL_WARN, "cannot invert option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

bool optionController::invalid(optionType opt) {
  switch(opt) {
    using enum optionType;
    case OPTION_SET_HAND_RANGE:
      [[fallthrough]];
    case OPTION_HAND_RANGE:
      return !get(OPTION_TRACK_DIVISION);
    case OPTION_SET_DARKEN_IMAGE:
      [[fallthrough]];
    case OPTION_DARKEN_IMAGE:
      return !ctr.image.exists();
    case OPTION_SHADOW_DISTANCE:
      return !get(OPTION_SHADOW);
    default:
      return false;
  }
}

void optionController::set(optionType opt, int value) {

  if (invalid(opt)) { return; }
  
  switch (opt) {
    using enum optionType;
    case OPTION_HAND_RANGE:
      [[fallthrough]];
    case OPTION_DARKEN_IMAGE:
      [[fallthrough]];
    case OPTION_CIE_FUNCTION:
      [[fallthrough]];
    case OPTION_SHADOW_DISTANCE:
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
