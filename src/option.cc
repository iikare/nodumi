#include "option.h"

optionController::optionController() {
  opts.resize(static_cast<int>(optionType::OPTION_NONE), 0);
}

void optionController::invert(optionType opt) {
  opts[static_cast<int>(opt)] = !opts[static_cast<int>(opt)];
}

bool optionController::get(optionType opt) {
  return opts[static_cast<int>(opt)];
}
