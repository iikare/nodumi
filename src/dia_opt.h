#pragma once

#include <vector>
#include <string>
#include "enum.h"
#include "log.h"

using std::vector;
using std::string;

class dialogOption {
  public:

    dialogOption() = delete;

    // basic check box
    dialogOption(DIA_OPT t, optionType opt_t, const vector<string>& label);
    
    // check box with subboxes
    dialogOption(DIA_OPT t, optionType opt_t, optionType sub_opt_t, const vector<string>& label, const vector<string> val) {
      if (t != DIA_OPT::SUBBOX) {
        logW(LL_CRIT, "invalid constructor for dialog option of type", t, "- expecting type", DIA_OPT::SUBBOX);
        return;
      }


      type = t;
      link_opt = opt_t;
      link_sub_opt = sub_opt_t;
      text = label;
      value = val;
      x = y = 0;

      if (label.size() < 1) {
        logW(LL_CRIT, "dialog option of type", t ,"has invalid label set");
      }
    }

    int render(int in_x, int in_y);
    void process();

  private:
  
    void renderBox();

    DIA_OPT type;
    optionType link_opt;
    optionType link_sub_opt;
    
    int x = 0;
    int y = 0;

    vector<string> text;
    vector<string> value;


    static constexpr int itemRectSize = 24;
    static constexpr int itemRectInnerSize = 12;
    static constexpr int itemFontSize = 18;

    static constexpr int boxOffset = itemRectSize + 4;
    static constexpr int boxW = 30;
    static constexpr int boxH = 30;
    static constexpr int boxSpacing = 5;
};
