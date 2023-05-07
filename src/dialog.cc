#include "dialog.h"

#include <string>

#include "define.h"
#include "data.h"
#include "enum.h"
#include "wrap.h"
#include "aghdef.h"

using std::string;
using std::to_string;

void dialogController::init() {
  dia_opts.push_back(dialogOption(DIA_OPT::CHECK_ONLY, 
                                  optionType::OPTION_TRACK_DIVISION, 
                                  ctr.text.getStringSet("PREF_TRACK_DIVIDE")
                                  ));

  dia_opts.push_back(dialogOption(DIA_OPT::SUBBOX, 
                                  optionType::OPTION_SET_HAND_RANGE,
                                  optionType::OPTION_HAND_RANGE,
                                  ctr.text.getStringSet("PREF_HAND_RANGE"),
                                  {"8","9","10","11"}
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

int dialogController::getItemX(int pos, bool box) const {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  return prefSideMargin/2.0f+prefItemXSpacing+(ctr.prefWidth/2.0f*pos) - 
         (box ? prefItemXSpacing/2-itemRectSize/2 : -(itemRectSize+4));
}

int dialogController::getItemY(int pos, bool box) const {
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  return prefTopMargin/2.0f + prefItemYSpacing*pos + 6 + (box ? itemFontSize/2.0f-itemRectSize/2.0f : 2) + 40;
}

void dialogController::process() {
  for (auto& d_opt : dia_opts) {
    d_opt.process();
  }
}

void dialogController::renderPreference() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  drawRectangle(prefSideMargin/2.0f, prefTopMargin/2.0f, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);  

  drawTextEx(ctr.text.getString("PREF_LABEL"), prefSideMargin/2.0f + 12, prefTopMargin/2.0f + 12, ctr.bgDark, 255, 24);
  
  int x_sum = getItemX(0, true);
  int y_sum = getItemY(0, true);

  for (auto& i : dia_opts) {
    y_sum += i.render(x_sum, y_sum);
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
