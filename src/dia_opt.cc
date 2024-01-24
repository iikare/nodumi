#include "dia_opt.h"

#include "box.h"
#include "build_target.h"
#include "define.h"
#include "lerp.h"
#include "wrap.h"

dialogOption::dialogOption(DIA_OPT t, OPTION opt_t,
                           const vector<string>& label) {
  if (t != DIA_OPT::CHECK_ONLY) {
    logW(LL_CRIT, "invalid constructor for dialog option of type", t,
         "- expecting type", DIA_OPT::CHECK_ONLY);
    return;
  }
  type = t;
  link_opt = opt_t;
  text = label;
  x = y = 0;

  if (label.size() < 1) {
    logW(LL_CRIT, "dialog option of type", t, "has invalid label set");
  }
}

dialogOption::dialogOption(DIA_OPT t, OPTION opt_t, OPTION sub_opt_t,
                           const vector<string>& label,
                           const vector<string>& val, const vector<int>& res) {
  if (!any_of(t, DIA_OPT::SUBBOX, DIA_OPT::SLIDER)) {
    logW(LL_CRIT, "invalid constructor for dialog option of type", t,
         "- expecting types", DIA_OPT::SUBBOX, "or", DIA_OPT::SLIDER);
    return;
  }

  if (label.size() < 1) {
    logW(LL_CRIT, "dialog option of type", type, "has invalid label set");
    return;
  }

  if (val.size() != res.size()) {
    logW(LL_CRIT, "inconsistent option result size:", val.size(), "v.",
         res.size());
  }

  type = t;
  link_opt = opt_t;
  link_sub_opt = sub_opt_t;
  text = label;
  value = val;
  result = res;
  x = y = 0;
}

void dialogOption::process() {
  if (ctr.option.invalid(link_opt)) {
    return;
  }
  if (pointInBox(getMousePosition(), {x, y, itemRectSize, itemRectSize})) {
    ctr.option.invert(link_opt);
  }

  int x_start = x + boxOffset;
  int y_start = y + boxOffset;

  if (type == DIA_OPT::SUBBOX && ctr.option.get(link_opt)) {
    for (unsigned int v = 0; v < value.size(); ++v) {
      if (pointInBox(getMousePosition(), {x_start, y_start, boxW, boxH})) {
        ctr.option.set(link_sub_opt, result[v]);
      }
      x_start += boxW + boxSpacing;
    }
  }
  else if (type == DIA_OPT::SLIDER && ctr.option.get(link_opt)) {
    constexpr int w = sliderLineSize;
    constexpr int y_space = sliderBoxSize / 2;

    if (pointInBox(getMousePosition(),
                   {x_start - y_space, y_start - y_space + 10, w + 2 * y_space,
                    2 * y_space})) {
      sliderActive = true;

      updateSliderValue();
    }
  }
}

void dialogOption::end_process() { sliderActive = false; }

int dialogOption::get_height() {
  switch (type) {
    using enum DIA_OPT;
    case CHECK_ONLY:
      return itemRectSize + 4;
    case SUBBOX:
      [[fallthrough]];
    case SLIDER:
      return itemRectSize + 4 + boxW + 4;
    default:
      return 0;
  }
}

void dialogOption::render(int in_x, int in_y) {
  if (sliderActive) {
    updateSliderValue();
  }

  x = in_x;
  y = in_y;

  bool opt_status = ctr.option.get(link_opt);
  bool inv_status = ctr.option.invalid(link_opt);

  auto col =
      inv_status ? ctr.bgMenuShade : (opt_status ? ctr.bgOpt : ctr.bgDark);

  drawRectangleLines(in_x, in_y, itemRectSize, itemRectSize, 3, col);

  if (opt_status) {
    drawRectangle(in_x + (itemRectSize - itemRectInnerSize) / 2.0f,
                  in_y + (itemRectSize - itemRectInnerSize) / 2.0f,
                  itemRectInnerSize, itemRectInnerSize, col);
  }
  switch (type) {
    using enum DIA_OPT;
    case SUBBOX:
      renderBox();
      break;
    case SLIDER:
      renderSlider();
      break;
    default:
      break;
  }

  // first element must exist
  drawTextEx(text[0], in_x + itemRectSize + 4, in_y + 6, ctr.bgDark, 255,
             itemFontSize);
}

void dialogOption::renderBox() {
  if (type != DIA_OPT::SUBBOX) {
    return;
  }

  int x_start = x + boxOffset;
  int y_start = y + boxOffset;

  constexpr int num_size = 15;

  bool opt_status = ctr.option.get(link_opt);

  for (unsigned int v = 0; v < value.size(); ++v) {
    //  8: 12
    //  9: 14
    // 10: 16
    // 11: 17

    bool opt_selected = result[v] == ctr.option.get(link_sub_opt);
    bool inv_status = ctr.option.invalid(link_opt);
    // logQ(conv_range,ctr.option.get(link_sub_opt));

    colorRGB& col = opt_selected ? ctr.bgMenu : ctr.bgDark;
    Vector2 rtSize = measureTextEx(value[v], num_size);
    float rtX = x_start + boxH / 2.0 - rtSize.x / 2 - 1;
    float rtY =
        y_start + boxH / 2.0 - rtSize.y / 2 + 1 + (result[v] == 12 ? 1 : 0);

    bool is_active = opt_status && !inv_status;

    if (opt_selected) {
      drawRectangle(x_start, y_start, boxW, boxH,
                    is_active ? ctr.bgOpt2 : ctr.bgMenuShade);
    }
    else {
      drawRectangleLines(x_start, y_start, boxW, boxH, 2,
                         is_active ? ctr.bgDark : ctr.bgMenuShade);
    }

    x_start += boxW + boxSpacing;

    drawTextEx(value[v], {rtX, rtY}, col, 255, num_size);
  }
}

void dialogOption::renderSlider() {
  int sub_opt_value = ctr.option.get(link_sub_opt);

  int x_start = x + boxOffset;
  int y_start = y + boxOffset + 10;

  constexpr int w = sliderLineSize;

  drawLineEx(x_start, y_start, x_start + w, y_start, 2, ctr.bgMenuShade);

  constexpr int n_div = 4;

  for (int i = 0; i <= n_div; ++i) {
    const int x_space = i * w / n_div;
    constexpr int y_space = sliderLineDashSize;

    drawLineEx(x_start + x_space, y_start - y_space, x_start + x_space,
               y_start + y_space, 2, ctr.bgMenuShade);
  }
  float p_ratio = static_cast<float>(sub_opt_value) / (result[1] - result[0]);
  constexpr int p_size = sliderBoxSize;

  const auto& col = ctr.option.get(link_opt) ? ctr.bgOpt2 : ctr.bgMenuShade;

  drawRectangle(x_start + w * p_ratio - p_size / 2.0f, y_start - p_size / 2.0f,
                p_size, p_size, col);
}

void dialogOption::updateSliderValue() {
  int x_start = x + boxOffset;
  constexpr int w = sliderLineSize;
  int n_pos = valueLERP(result[1], result[0],
                        static_cast<double>(ctr.getMouseX() - x_start) / w);
  n_pos = max(result[0], min(result[1], n_pos));
  ctr.option.set(link_sub_opt, n_pos);
}
