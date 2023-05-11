#include "dialog.h"

#include <string>

#include "define.h"
#include "data.h"
#include "enum.h"
#include "wrap.h"
#include "misc.h"
#include "aghdef.h"

using std::string;

void dialogController::init() {
  dia_opts.push_back(dialogOption(DIA_OPT::CHECK_ONLY, 
                                  optionType::OPTION_TRACK_DIVISION, 
                                  ctr.text.getStringSet("PREF_TRACK_DIVIDE")
                                  ));
  dia_opts.push_back(dialogOption(DIA_OPT::SUBBOX, 
                                  optionType::OPTION_SET_HAND_RANGE,
                                  optionType::OPTION_HAND_RANGE,
                                  ctr.text.getStringSet("PREF_HAND_RANGE"),
                                  {"8","9","10","11"},
                                  {12,14,16,17}
                                  ));
  dia_opts.push_back(dialogOption(DIA_OPT::CHECK_ONLY, 
                                  optionType::OPTION_DYNAMIC_LABEL,
                                  ctr.text.getStringSet("PREF_DYNAMIC_LABEL")
                                  ));
  dia_opts.push_back(dialogOption(DIA_OPT::SLIDER, 
                                  optionType::OPTION_SET_DARKEN_IMAGE, 
                                  optionType::OPTION_DARKEN_IMAGE, 
                                  ctr.text.getStringSet("PREF_IMAGE_DARKEN"),
                                  {"0","255"},
                                  {0,255}
                                  ));
  dia_opts.push_back(dialogOption(DIA_OPT::SUBBOX, 
                                  optionType::OPTION_SET_CIE_FUNCTION,
                                  optionType::OPTION_CIE_FUNCTION,
                                  ctr.text.getStringSet("PREF_CIE_FUNCTION"),
                                  {"00", "94", "76"},
                                  convertEnum(vector<decltype(CIE::C_00)>{CIE::C_00, CIE::C_94, CIE::C_76})
                                  ));
}

void dialogController::render() {
  if (preferenceDisplay) { 
    renderPreference();
  }
  if (infoDisplay) {
    renderInfo();
  }
}

int dialogController::getItemX(int pos) const {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  return prefSideMargin/2.0+prefItemXSpacing+(ctr.prefWidth/2.0*pos) - 
         prefItemXSpacing/2.0-itemRectSize/2.0 +
         ((pos == 0) ? 20 : 2);
}

int dialogController::getItemY(int pos) const {
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  return prefTopMargin/2.0 + prefItemYSpacing*pos + 6 + itemFontSize/2.0-itemRectSize/2.0 + 40;
}

void dialogController::process() {
  for (auto& d_opt : dia_opts) {
    d_opt.process();
  }
}

void dialogController::end_process() {
  for (auto& d_opt : dia_opts) {
    d_opt.end_process();
  }
}

void dialogController::renderPreference() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  drawRectangle(prefSideMargin/2.0f, prefTopMargin/2.0f, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);  

  drawTextEx(ctr.text.getString("PREF_LABEL"), prefSideMargin/2.0f + 12, prefTopMargin/2.0f + 12, ctr.bgDark, 255, 24);
  
  int x_sum = getItemX(0);
  int y_sum = getItemY(0);

  for (auto& i : dia_opts) {
    if (y_sum + i.get_height() > (ctr.getHeight()+ctr.prefHeight)/2.0 + optBottomMargin) {
      x_sum = getItemX(1);
      y_sum = getItemY(0);
      i.render(x_sum, y_sum);
    }
    else {
      i.render(x_sum, y_sum);
      y_sum += i.get_height(); 
    }

  }

}

void dialogController::renderInfo() {
  const int infoSideMargin = ctr.getWidth() - ctr.infoWidth;
  const int infoTopMargin = ctr.getHeight() - ctr.infoHeight;
  drawRectangle(infoSideMargin/2.0f, infoTopMargin/2.0f, ctr.infoWidth, ctr.infoHeight, ctr.bgMenu);  

  Vector2 iconTextVec = measureTextEx(W_NAME, iconTextSize);
  double iconTextHeight = iconTextVec.y;
  double iconBoxWidth = iconTextVec.x + ctr.getImage("ICON").width*iconScale +
                        borderMargin*2 - iconTextAdjust;
  double iconBoxHeight = ctr.getImage("ICON").height*iconScale + borderMargin;
  double iconTextX = infoSideMargin/2.0f + (ctr.infoWidth - iconBoxWidth)/2.0f + 
                     ctr.getImage("ICON").width*iconScale + borderMargin - iconTextAdjust;
  double iconTextY = infoTopMargin/2.0f + ctr.getImage("ICON").height*iconScale/2.0f + borderMargin - iconTextHeight/2.0f;

  drawTextEx(W_NAME, iconTextX, iconTextY, ctr.bgIcon, 255, iconTextSize);
  Vector2 iconPos = {static_cast<float>(infoSideMargin/2.0f + (ctr.infoWidth-iconBoxWidth)/2.0f + borderMargin), 
                     static_cast<float>(infoTopMargin/2.0f + borderMargin)};
  drawTextureEx(ctr.getImage("ICON"), iconPos, 0, 0.3);
  
  drawTextEx(ctr.text.getString("INFO_BOX_BUILD_DATE")+ " " + string(BUILD_DATE), 
             infoSideMargin/2.0f+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2.0f, ctr.bgDark);
  drawTextEx(ctr.text.getString("INFO_BOX_VER") + " " + string(W_VER), 
             infoSideMargin/2.0f+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2.0f+16, ctr.bgDark);
  
  double copySymWidth = measureTextEx(copySym, copySymSize).x;
  double copyWidth = measureTextEx(copy).x;
  double licenseWidth = measureTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3")).x;
  

  drawTextEx(copySym, infoSideMargin/2.0f+ctr.infoWidth-borderMargin-copySymWidth-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2.0f + 2, ctr.bgDark, 255, copySymSize);
  drawTextEx(copy, infoSideMargin/2.0f+ctr.infoWidth-borderMargin-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2.0f, ctr.bgDark);
  drawTextEx(ctr.text.getString("INFO_BOX_LICENSE_GPL3"), infoSideMargin/2.0f+ctr.infoWidth-borderMargin-licenseWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2+16, ctr.bgDark);
}

bool dialogController::hover() {

  const auto hoverDialog = [&](bool diaDisplay, int x, int y, int w, int h) {
    if (diaDisplay) {
      const rect boundingBox = { x, y, w, h };
      if(pointInBox(getMousePosition(), boundingBox)) {
        return true;
      }
    }
    return false;
  };
  return hoverDialog(preferenceDisplay, ctr.getWidth()/2  - ctr.prefWidth/2,
                                        ctr.getHeight()/2 - ctr.prefHeight/2,
                                        ctr.prefWidth,
                                        ctr.prefHeight) ||
         hoverDialog(infoDisplay,       ctr.getWidth()/2  - ctr.infoWidth/2,
                                        ctr.getHeight()/2 - ctr.infoHeight/2,
                                        ctr.infoWidth,
                                        ctr.infoHeight);
}
