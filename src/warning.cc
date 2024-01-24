#include "warning.h"

#include "define.h"
#include "wrap.h"

// requires packed executable data, must be loaded after `ctr.init()`
void warningController::init() {
#if defined(TARGET_WIN)
  win_warn_msg_size = measureTextEx(
      ctr.text.getString("WIN_WARN_MSG_EXPERIMENTAL"), win_warn_fsize);
#endif
}

void warningController::render() {
#if defined(TARGET_WIN)

  const rect win_warn_rect = {
      win_warn_offset,
      static_cast<int>(ctr.getHeight() - win_warn_offset - win_warn_msg_size.y -
                       win_warn_spacing),
      static_cast<int>(win_warn_msg_size.x + win_warn_spacing),
      static_cast<int>(win_warn_msg_size.y + win_warn_spacing)};

  const Vector2 win_warn_msg_loc = {win_warn_rect.x + win_warn_spacing / 2.0f,
                                    win_warn_rect.y + win_warn_spacing / 2.0f};

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
      pointInBox(getMousePosition(), win_warn_rect)) {
    win_warning = false;
  }
  if (win_warning) {
    drawRectangle(win_warn_rect.x, win_warn_rect.y, win_warn_rect.width,
                  win_warn_rect.height, {255, 112, 143});
    drawTextEx(ctr.text.getString("WIN_WARN_MSG_EXPERIMENTAL"),
               win_warn_msg_loc, {255, 255, 255}, 255, win_warn_fsize);
  }
#endif
}
