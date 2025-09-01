#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include "enum.h"
#include "log.h"

using std::string;
using std::vector;

class dialogOption {
 public:
  dialogOption() = delete;

  // basic check box
  dialogOption(DIA_OPT t, OPTION opt_t, const vector<string>& label);

  // check box with subboxes OR slider
  dialogOption(DIA_OPT t, OPTION opt_t, OPTION sub_opt_t, const vector<string>& label, const vector<string>& val,
               const vector<int>& res, bool skip_main_box = false);

  void render(int in_x, int in_y);

  int get_height();

  void process();
  void end_process();

 private:
  void renderBox();
  void renderSlider();

  void updateSliderValue();

  DIA_OPT type;
  OPTION link_opt;
  OPTION link_sub_opt;

  bool sliderActive = false;
  bool skip_main_box = false;

  int x = 0;
  int y = 0;

  vector<string> text;
  vector<string> value;
  vector<int> result;

  static constexpr int itemRectSize = 24;
  static constexpr int itemRectInnerSize = 12;
  static constexpr int itemFontSize = 18;

  static constexpr int boxOffset = itemRectSize + 4;
  static constexpr int boxW = 30;
  static constexpr int boxH = 30;
  static constexpr int boxSpacing = 5;

  static constexpr int sliderLineSize = 180;
  static constexpr int sliderLineDashSize = 4;
  static constexpr int sliderBoxSize = 12;
};
