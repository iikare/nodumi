#include "option.h"

#include "cie2k.h"
#include "data.h"
#include "define.h"
#include "enum.h"

optionController::optionController() {
  opts.resize(static_cast<int>(OPTION::NONE), 0);
  opts[static_cast<int>(OPTION::TRACK_DIVISION_LIVE)] = true;
  opts[static_cast<int>(OPTION::TRACK_DIVISION_MIDI)] = true;
  opts[static_cast<int>(OPTION::SET_HAND_RANGE)] = false;
  opts[static_cast<int>(OPTION::HAND_RANGE)] = MAX_HAND_RANGE;
  opts[static_cast<int>(OPTION::SET_DARKEN_IMAGE)] = false;
  opts[static_cast<int>(OPTION::CIE_FUNCTION)] = static_cast<int>(cie2k::TYPE::CIE_00);
  opts[static_cast<int>(OPTION::SET_CIE_FUNCTION)] = false;
  opts[static_cast<int>(OPTION::DARKEN_IMAGE)] = 255;
  opts[static_cast<int>(OPTION::DYNAMIC_LABEL)] = true;
  opts[static_cast<int>(OPTION::PARTICLE)] = false;
  opts[static_cast<int>(OPTION::SCALE_VELOCITY)] = false;
  opts[static_cast<int>(OPTION::SHADOW)] = false;
  opts[static_cast<int>(OPTION::SHADOW_DISTANCE)] = 8;
  opts[static_cast<int>(OPTION::SHADOW_ANGLE)] = 135;
  opts[static_cast<int>(OPTION::NOW_LINE_USE_OVERLAY)] = false;
  opts[static_cast<int>(OPTION::USE_LEVEL_CONTROL)] = false;
  opts[static_cast<int>(OPTION::LEVEL_CONTROL_R)] = 255;
  opts[static_cast<int>(OPTION::LEVEL_CONTROL_G)] = 255;
  opts[static_cast<int>(OPTION::LEVEL_CONTROL_B)] = 255;
  opts[static_cast<int>(OPTION::LIMIT_FPS)] = 1;
}

void optionController::invert(OPTION opt) {
  if (invalid(opt)) {
    return;
  }

  switch (opt) {
    case OPTION::TRACK_DIVISION_LIVE:
      if (ctr.getLiveState() && !get(opt)) {
        getColorScheme(2, ctr.setTrackOn, ctr.setTrackOff);
      }
      break;
    case OPTION::TRACK_DIVISION_MIDI:
      break;
    case OPTION::DYNAMIC_LABEL:
      ctr.optimizeBGColor(true);
      break;
    case OPTION::SET_HAND_RANGE:
      break;
    case OPTION::SET_DARKEN_IMAGE:
      ctr.setShaderValue("SH_LEVEL", "use_bw", static_cast<int>(!opts[static_cast<int>(opt)]));
      break;
    case OPTION::SET_CIE_FUNCTION:
      break;
    case OPTION::PARTICLE:
      break;
    case OPTION::SCALE_VELOCITY:
      break;
    case OPTION::SHADOW:
      break;
    case OPTION::LIMIT_FPS:
      ctr.updateFPSCap(get(opt));
      break;
    case OPTION::NOW_LINE_USE_OVERLAY:
      ctr.setShaderValue("SH_OVERLAY", "use_line_color", static_cast<int>(!opts[static_cast<int>(opt)]));
      break;
    case OPTION::USE_LEVEL_CONTROL:
      ctr.setShaderValue("SH_LEVEL", "use_rgb", static_cast<int>(!opts[static_cast<int>(opt)]));
      break;
    default:
      logW(LL_WARN, "cannot invert option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

bool optionController::invalid(OPTION opt) {
  switch (opt) {
    case OPTION::SET_HAND_RANGE:
      [[fallthrough]];
    case OPTION::HAND_RANGE:
      return !get(OPTION::TRACK_DIVISION_LIVE) && !get(OPTION::TRACK_DIVISION_MIDI);
    case OPTION::SET_DARKEN_IMAGE:
      [[fallthrough]];
    case OPTION::DARKEN_IMAGE:
      return !ctr.image.exists();
    case OPTION::SHADOW_DISTANCE:
      return !get(OPTION::SHADOW);
    case OPTION::SHADOW_ANGLE:
      return !get(OPTION::SHADOW);
    case OPTION::LEVEL_CONTROL_R:
      [[fallthrough]];
    case OPTION::LEVEL_CONTROL_G:
      [[fallthrough]];
    case OPTION::LEVEL_CONTROL_B:
      return !get(OPTION::USE_LEVEL_CONTROL);
    default:
      return false;
  }
}

void optionController::set(OPTION opt, int value) {
  if (invalid(opt)) {
    return;
  }

  switch (opt) {
    case OPTION::HAND_RANGE:
      [[fallthrough]];
    case OPTION::CIE_FUNCTION:
      [[fallthrough]];
    case OPTION::SHADOW_DISTANCE:
      [[fallthrough]];
    case OPTION::SHADOW_ANGLE:
      break;
    case OPTION::SET_DARKEN_IMAGE:
      ctr.setShaderValue("SH_LEVEL", "use_bw", value);
      break;
    case OPTION::DARKEN_IMAGE:
      ctr.setShaderValue("SH_LEVEL", "thr_bw", static_cast<float>(value) / 255);
      break;
    case OPTION::LEVEL_CONTROL_R:
      ctr.setShaderValue("SH_LEVEL", "thr_r", static_cast<float>(value) / 255);
      break;
    case OPTION::LEVEL_CONTROL_G:
      ctr.setShaderValue("SH_LEVEL", "thr_g", static_cast<float>(value) / 255);
      break;
    case OPTION::LEVEL_CONTROL_B:
      ctr.setShaderValue("SH_LEVEL", "thr_b", static_cast<float>(value) / 255);
      break;
    case OPTION::USE_LEVEL_CONTROL:
      ctr.setShaderValue("SH_LEVEL", "use_rgb", value);
      break;
    case OPTION::NOW_LINE_USE_OVERLAY:
      ctr.setShaderValue("SH_OVERLAY", "use_line_color", value);
      break;
    default:
      logW(LL_WARN, "cannot modify option of type", static_cast<int>(opt));
      return;
  }

  opts[static_cast<int>(opt)] = value;
}

int optionController::get(OPTION opt) { return opts[static_cast<int>(opt)]; }
