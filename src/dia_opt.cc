#include "dia_opt.h"
#include "define.h"
#include "wrap.h"
#include "box.h"

dialogOption::dialogOption(DIA_OPT t, optionType opt_t, const vector<string>& label) {
  if (t != DIA_OPT::CHECK_ONLY) {
    logW(LL_CRIT, "invalid constructor for dialog option of type", t, "- expecting type", DIA_OPT::CHECK_ONLY);
    return;
  }
  type = t;
  link_opt = opt_t;
  text = label; 
  x = y = 0;

  if (label.size() < 1) {
    logW(LL_CRIT, "dialog option of type", t ,"has invalid label set");
  }

}

void dialogOption::process() {
  if (pointInBox(getMousePosition(), {x, y, itemRectSize,itemRectSize})) {
    ctr.option.invert(link_opt);
  }

  if (type == DIA_OPT::SUBBOX) {
    int x_start = x + boxOffset;
    int y_start = y + boxOffset;
    for (const auto& v: value) {
      if (pointInBox(getMousePosition(), {x_start, y_start, boxW, boxH})) {
        int conv_range = 12+2*(stoi(v)-8) + (stoi(v)==11 ? -1 : 0);
        ctr.option.set(link_sub_opt, conv_range);
      }
      x_start += boxW + boxSpacing;
    }
  }

}

int dialogOption::render(int in_x, int in_y) {

  x = in_x;
  y = in_y;

  bool opt_status = ctr.option.get(link_opt);

  auto col = opt_status ? ctr.bgOpt : ctr.bgDark;

  drawRectangleLines(in_x, in_y, itemRectSize,itemRectSize, 3, col);
  
  if (opt_status) {
    drawRectangle(in_x+(itemRectSize-itemRectInnerSize)/2.0f, 
                  in_y+(itemRectSize-itemRectInnerSize)/2.0f, 
                  itemRectInnerSize,itemRectInnerSize, col);
  
    switch (type) {
      case DIA_OPT::SUBBOX:
        renderBox();
        break;
      case DIA_OPT::SLIDER:
        break;
      default:
        break;
    }
  }

  // first element must exist
  drawTextEx(text[0],
             in_x + itemRectSize + 4,
             in_y + 6,
             ctr.bgDark, 255, itemFontSize);

  switch (type) {
    case DIA_OPT::CHECK_ONLY:
      return itemRectSize + 4;
    case DIA_OPT::SLIDER:
      break;
    case DIA_OPT::SUBBOX:
      return itemRectSize + 4 + boxW + 4;
    default:
      break;
  }
  return -1;
}

void dialogOption::renderBox() {
  if (type != DIA_OPT::SUBBOX) { return; }

  int x_start = x + boxOffset;
  int y_start = y + boxOffset;

  constexpr int num_size = 15;

  for (const auto& v : value) {

    //  8: 12
    //  9: 14
    // 10: 16
    // 11: 17
    int conv_range = 12+2*(stoi(v)-8) + (stoi(v)==11 ? -1 : 0);

    bool opt_selected = conv_range == ctr.option.get(link_sub_opt);
    //logQ(conv_range,ctr.option.get(link_sub_opt));

    colorRGB& col = opt_selected ? ctr.bgMenu : ctr.bgDark;
    Vector2 rtSize = measureTextEx(v, num_size);
    float rtX = x_start+boxH/2.0-rtSize.x/2 - 1;
    float rtY = y_start+boxH/2.0-rtSize.y/2 + 1 + (conv_range == 12 ? 1 : 0);

    if (opt_selected) {
      drawRectangle(x_start, y_start, boxW, boxH, ctr.bgOpt2);
    }
    else {
      drawRectangleLines(x_start, y_start, boxW, boxH, 2, ctr.bgDark);
    }

    x_start += boxW + boxSpacing;
    
    drawTextEx(v, {rtX, rtY}, col, 255, num_size);

  }

}
