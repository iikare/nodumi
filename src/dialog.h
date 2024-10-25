#pragma once

#include <string>
#include <unordered_map>

#include "aghdef.h"
#include "dia_opt.h"
#include "enum.h"
#include "log.h"

using std::string;
using std::unordered_map;

class dialogController {
 public:
  void init();

  void render();

  void process();
  void end_process();

  bool hover();

  bool get_status(DIALOG d) const;
  void set_status(DIALOG d, bool value);
  void invert_status(DIALOG d);
  void clear_status();
  void clear_invert_status(DIALOG d);

  PREF c_pref_t = PREF::P1;

 private:
  unordered_map<PREF, vector<dialogOption>> dia_opts;
  unordered_map<PREF, string> dia_menu_label = {
      {PREF::P1, "PREF_P1"},
      {PREF::P2, "PREF_P2"},
  };

  vector<bool> dialog_status = {false, false, false};

  void renderPreference();
  void renderFile();
  void renderInfo();

  int getItemX(int pos) const;
  int getItemY(int pos) const;

  static constexpr int iconTextSize = 30;
  static constexpr int iconTextAdjust = 10;
  static constexpr double borderMargin = 20;
  static constexpr double iconScale = 0.3;
  const string copySym = "©";
  const string copy = " iika-re 2020-" + string(COPY_YEAR);

  static constexpr int copySymSize = 22;

  static constexpr int prefItemXSpacing = 30;
  static constexpr int prefItemYSpacing = 28;
  static constexpr int itemRectSize = 24;
  static constexpr int itemRectInnerSize = 12;
  static constexpr int itemFontSize = 18;

  static constexpr int optBottomMargin = 10;
};
