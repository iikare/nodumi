#include "dialog.h"

#include <string>

#include "aghdef.h"
#include "cie2k.h"
#include "data.h"
#include "define.h"
#include "enum.h"
#include "menu.h"
#include "misc.h"
#include "wrap.h"

using std::fill;
using std::string;

void dialogController::init() {
  for (unsigned int n = 0; n < static_cast<int>(PREF::NONE); ++n) {
    dia_opts.insert(make_pair(static_cast<PREF>(n), vector<dialogOption>{}));
  }
  dialog_status.resize(static_cast<int>(PREF::NONE), false);

  dia_opts.find(PREF::P1)->second.push_back(dialogOption(DIA_OPT::CHECK_ONLY, OPTION::SCALE_VELOCITY,
                                                         ctr.text.getStringSet("PREF_SCALE_VELOCITY")));
  dia_opts.find(PREF::P1)->second.push_back(dialogOption(DIA_OPT::CHECK_ONLY, OPTION::TRACK_DIVISION_MIDI,
                                                         ctr.text.getStringSet("PREF_TRACK_DIVIDE_MIDI")));
  dia_opts.find(PREF::P1)->second.push_back(dialogOption(DIA_OPT::CHECK_ONLY, OPTION::TRACK_DIVISION_LIVE,
                                                         ctr.text.getStringSet("PREF_TRACK_DIVIDE_LIVE")));
  dia_opts.find(PREF::P1)->second.push_back(
      dialogOption(DIA_OPT::SUBBOX, OPTION::SET_HAND_RANGE, OPTION::HAND_RANGE,
                   ctr.text.getStringSet("PREF_HAND_RANGE"), {"8", "9", "10", "11"}, {12, 14, 16, 17}));
  dia_opts.find(PREF::P1)->second.push_back(
      dialogOption(DIA_OPT::CHECK_ONLY, OPTION::USE_LSTM, ctr.text.getStringSet("PREF_USE_LSTM")));
  dia_opts.find(PREF::P1)->second.push_back(
      dialogOption(DIA_OPT::CHECK_ONLY, OPTION::LIMIT_FPS, ctr.text.getStringSet("PREF_LIMIT_FPS")));

  dia_opts.find(PREF::P2)->second.push_back(
      dialogOption(DIA_OPT::CHECK_ONLY, OPTION::PARTICLE, ctr.text.getStringSet("PREF_PARTICLE")));
  dia_opts.find(PREF::P2)->second.push_back(
      dialogOption(DIA_OPT::CHECK_ONLY, OPTION::DYNAMIC_LABEL, ctr.text.getStringSet("PREF_DYNAMIC_LABEL")));
  dia_opts.find(PREF::P2)->second.push_back(dialogOption(
      OPTION::SHADOW, {OPTION::SHADOW_DISTANCE, OPTION::SHADOW_ANGLE},
      ctr.text.getStringSet("PREF_SHADOW", "PREF_ANGLE"), {{"0", "20"}, {"0", "360"}}, {{0, 20}, {0, 360}}));
  dia_opts.find(PREF::P2)->second.push_back(dialogOption(DIA_OPT::CHECK_ONLY, OPTION::NOW_LINE_USE_OVERLAY,
                                                         ctr.text.getStringSet("PREF_NOW_LINE_USE_OVERLAY")));

  dia_opts.find(PREF::P3)->second.push_back(dialogOption(OPTION::SET_DARKEN_IMAGE, {OPTION::DARKEN_IMAGE},
                                                         ctr.text.getStringSet("PREF_IMAGE_DARKEN"),
                                                         {{"0", "255"}}, {{0, 255}}));
  auto cie_opt_vec = vector<cie2k::TYPE>{cie2k::TYPE::CIE_00, cie2k::TYPE::CIE_94, cie2k::TYPE::CIE_76};
  dia_opts.find(PREF::P3)->second.push_back(
      dialogOption(DIA_OPT::SUBBOX, OPTION::SET_CIE_FUNCTION, OPTION::CIE_FUNCTION,
                   ctr.text.getStringSet("PREF_CIE_FUNCTION"), {"00", "94", "76"}, convertEnum(cie_opt_vec)));
  dia_opts.find(PREF::P3)->second.push_back(dialogOption(
      OPTION::USE_LEVEL_CONTROL, {OPTION::LEVEL_CONTROL_R, OPTION::LEVEL_CONTROL_G, OPTION::LEVEL_CONTROL_B},
      ctr.text.getStringSet("PREF_LEVEL_CONTROL"), {{"0", "255"}, {"0", "255"}, {"0", "255"}},
      {{0, 255}, {0, 255}, {0, 255}}));
}

void dialogController::render() {
  if (get_status(DIALOG::PREFERENCES)) {
    renderPreference();
  }
  if (get_status(DIALOG::FILE)) {
    renderFile();
  }
  if (get_status(DIALOG::INFO)) {
    renderInfo();
  }
}

int dialogController::getItemX(int pos) const {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  return prefSideMargin / 2.0 + prefItemXSpacing + (ctr.prefWidth / 2.0 * pos) - prefItemXSpacing / 2.0 -
         itemRectSize / 2.0 + ((pos == 0) ? 20 : 2);
}

int dialogController::getItemY(int pos) const {
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  return prefTopMargin / 2.0 + prefItemYSpacing * pos + 6 + itemFontSize / 2.0 - itemRectSize / 2.0 + 40;
}

void dialogController::process() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  int in_x = prefSideMargin / 2.0f + 12 + measureTextEx(ctr.text.getString("PREF_LABEL"), 24).x + 8;
  int in_y = prefTopMargin / 2.0f + 10;

  for (unsigned int n = 0; n < static_cast<int>(PREF::NONE); ++n) {
    if (pointInBox(getMousePosition(), {in_x, in_y, itemRectSize, itemRectSize})) {
      c_pref_t = static_cast<PREF>(n);
      break;
    }
    in_x += itemRectSize + 3;
  }

  for (auto& d_opt : dia_opts.find(c_pref_t)->second) {
    d_opt.process();
  }
}

void dialogController::end_process() {
  for (auto& d_opt : dia_opts.find(c_pref_t)->second) {
    d_opt.end_process();
  }
}

void dialogController::renderPreference() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  drawRectangle(prefSideMargin / 2.0f, prefTopMargin / 2.0f, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);

  const int pref_fsize = 25;
  drawTextEx(ctr.text.getString("PREF_LABEL"), prefSideMargin / 2.0f + 12, prefTopMargin / 2.0f + 12,
             ctr.bgDark, 255, pref_fsize);

  string menu_label = ctr.text.getString(dia_menu_label.find(c_pref_t)->second);
  // Vector2 menu_label_size = measureTextEx(menu_label);

  int in_x = prefSideMargin / 2.0f + 12 + measureTextEx(ctr.text.getString("PREF_LABEL"), pref_fsize).x + 8;
  int in_y = prefTopMargin / 2.0f + 9;
  // int in_l_x = (ctr.getWidth() + ctr.prefWidth)/2.0f - menu_label_size.x -
  // 12;
  int in_l_y = in_y + 6;

  for (unsigned int n = 0; n < static_cast<int>(PREF::NONE); ++n) {
    bool active = static_cast<PREF>(n) == c_pref_t;
    auto col = active ? ctr.bgOpt2 : ctr.bgDark;

    drawRectangleLines(in_x, in_y, itemRectSize, itemRectSize, 3, col);
    if (active) {
      drawRectangle(in_x + (itemRectSize - itemRectInnerSize) / 2.0f,
                    in_y + (itemRectSize - itemRectInnerSize) / 2.0f, itemRectInnerSize, itemRectInnerSize,
                    col);
    }
    in_x += itemRectSize + 3;
  }
  drawTextEx(menu_label, in_x, in_l_y, ctr.bgDark);

  int x_sum = getItemX(0);
  int y_sum = getItemY(0);

  for (auto& i : dia_opts.find(c_pref_t)->second) {
    if (y_sum + i.get_height() > (ctr.getHeight() + ctr.prefHeight) / 2.0 - optBottomMargin) {
      x_sum = getItemX(1);
      y_sum = getItemY(0);
    }
    i.render(x_sum, y_sum);
    y_sum += i.get_height();
  }
}

void dialogController::renderFile() {
  const int fileSideMargin = ctr.getWidth() - ctr.fileWidth;
  const int fileTopMargin = ctr.getHeight() - ctr.fileHeight;
  drawRectangle(fileSideMargin / 2.0f, fileTopMargin / 2.0f, ctr.fileWidth, ctr.fileHeight, ctr.bgMenu);

  int begin_x = fileSideMargin / 2.0f + 12;
  int begin_y = fileTopMargin / 2.0f + 12;
  string file_label_str = ctr.text.getString("FILE_INFO_LABEL");
  const int file_label_fsize = 25;
  Vector2 file_label_size = measureTextEx(file_label_str, file_label_fsize);
  drawTextEx(file_label_str, begin_x, begin_y, ctr.bgDark, 255, file_label_fsize);

  vector<string> file_labels = {
      "FILE_TYPE",
      "FILE_NOTE_COUNT",
      "FILE_MEASURE_COUNT",
      "FILE_TRACK_COUNT",
  };

  vector<pair<string, string>> file_infos;
  for (const auto& i : file_labels) {
    string i_res = "";
    if (i == "FILE_TYPE") {
      if (ctr.getLiveState()) {
        i_res = "N/A (live)";
      }
      else {
        fileType ft = ctr.getFileType();
        switch (ft) {
          case FILE_MIDI:
            i_res = "MIDI";
            break;
          case FILE_MKI:
            i_res = "MKI";
            break;
          default:
            break;
        }
      }
    }
    else if (i == "FILE_NOTE_COUNT") {
      i_res = to_string(ctr.getNoteCount());
    }
    else if (i == "FILE_MEASURE_COUNT") {
      i_res = to_string(ctr.getMeasureCount());
    }
    else if (i == "FILE_TRACK_COUNT") {
      i_res = to_string(ctr.getTrackCount());
    }
    file_infos.push_back(make_pair(ctr.text.getString(i), i_res));
  }

  begin_y += file_label_size.y + 4;

  for (const auto& i : file_infos) {
    Vector2 if_size = measureTextEx(i.first, itemFontSize);
    drawTextEx(i.first, begin_x, begin_y, ctr.bgDark, 255, itemFontSize);

    Vector2 is_size = measureTextEx(i.second, itemFontSize);
    int begin_s_x = ctr.getWidth() / 2.0f - 4 - is_size.x;
    drawTextEx(i.second, begin_s_x, begin_y, ctr.bgDark, 255, itemFontSize);

    begin_y += max(if_size.y, is_size.y) + 4;
  }
}

void dialogController::renderInfo() {
  const int infoSideMargin = ctr.getWidth() - ctr.infoWidth;
  const int infoTopMargin = ctr.getHeight() - ctr.infoHeight;
  drawRectangle(infoSideMargin / 2.0f, infoTopMargin / 2.0f, ctr.infoWidth, ctr.infoHeight, ctr.bgMenu);

  Vector2 iconTextVec = measureTextEx(W_NAME, iconTextSize);
  double iconTextHeight = iconTextVec.y;
  double iconBoxWidth =
      iconTextVec.x + ctr.getImage("ICON").width * iconScale + borderMargin * 2 - iconTextAdjust;
  // double iconBoxHeight = ctr.getImage("ICON").height*iconScale +
  // borderMargin;
  double iconTextX = infoSideMargin / 2.0f + (ctr.infoWidth - iconBoxWidth) / 2.0f +
                     ctr.getImage("ICON").width * iconScale + borderMargin - iconTextAdjust;
  double iconTextY = infoTopMargin / 2.0f + ctr.getImage("ICON").height * iconScale / 2.0f + borderMargin -
                     iconTextHeight / 2.0f;

  drawTextEx(W_NAME, iconTextX, iconTextY, ctr.bgIcon, 255, iconTextSize);
  Vector2 iconPos = {
      static_cast<float>(infoSideMargin / 2.0f + (ctr.infoWidth - iconBoxWidth) / 2.0f + borderMargin),
      static_cast<float>(infoTopMargin / 2.0f + borderMargin)};
  drawTextureEx(ctr.getImage("ICON"), iconPos, 0, 0.3);

  string build_deps = ctr.text.getString("INFO_BOX_BUILD_FLAGS") + " " +
                      string(ctr.input.lstm_enabled() ? "+" : "-") + "torch";
  double build_deps_height = measureTextEx(build_deps).y;
  drawTextEx(build_deps, infoSideMargin / 2.0f + borderMargin,
             infoTopMargin / 2.0f + ctr.infoHeight - build_deps_height - borderMargin - 22, ctr.bgDark);

  double copySymWidth = measureTextEx(copySym, copySymSize).x;
  double copyWidth = measureTextEx(copy).x;
  double copyHeight = measureTextEx(copy).y;
  double licenseWidth = measureTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3")).x;

  drawTextEx(ctr.text.getString("INFO_BOX_BUILD_DATE") + " " + string(BUILD_DATE),
             infoSideMargin / 2.0f + borderMargin, infoTopMargin / 2.0f + ctr.infoHeight - borderMargin - 20,
             ctr.bgDark);

  auto versionString = ctr.text.getString("INFO_BOX_VER") + " " + string(W_VER);
#if !defined(TARGET_REL)
  versionString += " - " + string(COMMIT_HASH);
#endif

  drawTextEx(versionString, infoSideMargin / 2.0f + borderMargin,
             infoTopMargin / 2.0f + ctr.infoHeight + copyHeight - borderMargin - 20, ctr.bgDark);

  drawTextEx(copySym, infoSideMargin / 2.0f + ctr.infoWidth - borderMargin - copySymWidth - copyWidth,
             infoTopMargin / 2.0f + ctr.infoHeight - borderMargin - 20, ctr.bgDark, 255, copySymSize);
  drawTextEx(copy, infoSideMargin / 2.0f + ctr.infoWidth - borderMargin - copyWidth,
             infoTopMargin / 2.0f + ctr.infoHeight - borderMargin - 20, ctr.bgDark);
  drawTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3"),
             infoSideMargin / 2.0f + ctr.infoWidth - borderMargin - licenseWidth,
             infoTopMargin / 2.0f + ctr.infoHeight + copyHeight - borderMargin - 20, ctr.bgDark);
}

bool dialogController::hover() {
  const auto hoverDialog = [&](bool diaDisplay, int x, int y, int w, int h) {
    if (diaDisplay) {
      const rect boundingBox = {x, y, w, h};
      if (pointInBox(getMousePosition(), boundingBox)) {
        return true;
      }
    }
    return false;
  };
  return hoverDialog(get_status(DIALOG::PREFERENCES), ctr.getWidth() / 2 - ctr.prefWidth / 2,
                     ctr.getHeight() / 2 - ctr.prefHeight / 2, ctr.prefWidth, ctr.prefHeight) ||
         hoverDialog(get_status(DIALOG::FILE), ctr.getWidth() / 2 - ctr.fileWidth / 2,
                     ctr.getHeight() / 2 - ctr.fileHeight / 2, ctr.fileWidth, ctr.fileHeight) ||
         hoverDialog(get_status(DIALOG::INFO), ctr.getWidth() / 2 - ctr.infoWidth / 2,
                     ctr.getHeight() / 2 - ctr.infoHeight / 2, ctr.infoWidth, ctr.infoHeight);
}

bool dialogController::get_status(DIALOG d) const {
  if (d == DIALOG::NONE) {
    logW(LL_WARN, "invalid dialog option:", static_cast<int>(d));
    return false;
  }
  return dialog_status[static_cast<int>(d)];
}

void dialogController::set_status(DIALOG d, bool value) {
  if (d == DIALOG::NONE) {
    logW(LL_WARN, "invalid dialog option:", static_cast<int>(d));
  }
  dialog_status[static_cast<int>(d)] = value;
}

void dialogController::invert_status(DIALOG d) {
  if (d == DIALOG::NONE) {
    logW(LL_WARN, "invalid dialog option:", static_cast<int>(d));
  }
  dialog_status[static_cast<int>(d)] = !dialog_status[static_cast<int>(d)];
}

void dialogController::clear_status() { fill(dialog_status.begin(), dialog_status.end(), false); }

void dialogController::clear_invert_status(DIALOG d) {
  bool d_o = get_status(d);
  clear_status();
  set_status(d, !d_o);
}
