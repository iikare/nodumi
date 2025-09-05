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
  dialogOption(DIA_OPT t, OPTION opt_t, OPTION sub_opt_t, const vector<string>& label,
               const vector<string>& val, const vector<int>& res);

  // multiple sliders (implicitly slider w/one parent condition)
  dialogOption(OPTION opt_t, vector<OPTION> sub_opts_t, const vector<string>& label,
               const vector<pair<string, string>>& val, const vector<pair<int, int>>& res);

  void render(int in_x, int in_y);

  int get_height();

  void process();
  void end_process();

 private:
  void renderBox();
  void renderSlider();

  void updateSliderValue(int index = -1);

  DIA_OPT type;
  OPTION link_opt;
  OPTION link_sub_opt;
  vector<OPTION> link_sub_opt_set;

  bool sliderActive = false;
  int sliderIndex = -1;

  int x = 0;
  int y = 0;

  vector<string> text;
  vector<string> value;
  vector<int> result;

  vector<pair<string, string>> value_set;
  vector<pair<int, int>> result_set;

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
  static constexpr int sliderVerticalSpacing = 20;
};
