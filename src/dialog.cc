#include "dialog.h"

#include <string>

#include "define.h"
#include "data.h"
#include "enum.h"
#include "wrap.h"
#include "aghdef.h"

using std::string;
using std::to_string;

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
  // entered while processing left click event
  if (pointInBox(getMousePosition(), {getItemX(0, true), getItemY(0, true), itemRectSize, itemRectSize})) {
    ctr.option.invert(optionType::OPTION_TRACK_DIVISION);
  }
  if (pointInBox(getMousePosition(), {getItemX(0, true), getItemY(1, true), itemRectSize, itemRectSize})) {
    ctr.option.invert(optionType::OPTION_SET_HAND_RANGE);
  }
  for (int i = 0; i < hrs_max_item; ++i) {
    if (pointInBox(getMousePosition(), {getHRSX(i), getItemY(2), hrs_boxW, hrs_boxH})) {
      ctr.option.set(optionType::OPTION_HAND_RANGE, getHRSValue(i));
      break;
    }
  }
}

void dialogController::renderPreference() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  drawRectangle(prefSideMargin/2.0f, prefTopMargin/2.0f, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);  

  drawTextEx("Preferences", prefSideMargin/2.0f + 12, prefTopMargin/2.0f + 12, ctr.bgDark, 255, 24);
  

  auto drawItem = [&] (string text, int x,  int y, optionType link_opt) {

    bool opt_status = ctr.option.get(link_opt);

    auto col = opt_status ? ctr.bgOpt : ctr.bgDark;

    drawRectangleLines(getItemX(x, true), getItemY(y, true), itemRectSize,itemRectSize, 3, col);
    
    if (opt_status) {
      drawRectangle(getItemX(x, true)+(itemRectSize-itemRectInnerSize)/2.0f, 
                    getItemY(y, true)+(itemRectSize-itemRectInnerSize)/2.0f, 
                    itemRectInnerSize,itemRectInnerSize, col);
    }

    drawTextEx(text,
               getItemX(x),
               getItemY(y),
               ctr.bgDark, 255, itemFontSize);
  };

  drawItem("Adaptive Track Division", 0, 0, optionType::OPTION_TRACK_DIVISION);
  
  drawItem("Custom Hand Range", 0, 1, optionType::OPTION_SET_HAND_RANGE);
  //drawItem("Adaptive Track Division", 0, 1, optionType::OPTION_TRACK_DIVISION);

  //logQ(ctr.option.get(optionType::OPTION_HAND_RANGE));

  if (ctr.option.get(optionType::OPTION_SET_HAND_RANGE)) {
    renderHandRangeSelector();
  }
}

int dialogController::getHRSX(int pos) const {
  return getItemX(0)+ pos*(hrs_boxSpacing + hrs_boxW) + hrs_boxSpacing; 
};
    
int dialogController::getHRSValue(int index) const {
  return 2*(index+8)-4-(index == 3 ? 1 : 0); 
}

void dialogController::renderHandRangeSelector() {


  constexpr int supscr_size = 10;
  constexpr int num_size = 15;



  for (int i = 0; i < 4; ++i) {

    bool opt_selected = getHRSValue(i) == ctr.option.get(optionType::OPTION_HAND_RANGE);

    colorRGB& col = opt_selected ? ctr.bgMenu : ctr.bgDark;

    string rangeText = to_string(i+8);

    Vector2 rtSize = measureTextEx(rangeText, num_size);
    Vector2 scrSize = measureTextEx("th", supscr_size);
    float rtX = getHRSX(i)+hrs_boxH/2.0-rtSize.x/2 - scrSize.x/2 + (i >= 2 ? 1 : 0); 
    float rtY = getItemY(2)+hrs_boxH/2.0-rtSize.y/2 + 2;
    
    if (opt_selected) {
      drawRectangle(getHRSX(i), getItemY(2), hrs_boxW, hrs_boxH, ctr.bgOpt2);
    }
    else {
      drawRectangleLines(getHRSX(i), getItemY(2), hrs_boxW, hrs_boxH, 2, ctr.bgDark);
    }

    drawTextEx("th", {rtX+rtSize.x+1 - (i>=2 ? i-1 : 0), getItemY(2)+6.0f}, col, 255, supscr_size);
    drawTextEx(rangeText, {rtX, rtY + (i==1 ? -1 : 0)}, col, 255, num_size);

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
  
  drawTextEx(string("Build Date: ") + BUILD_DATE, 
             infoSideMargin/2.0f+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2.0f, ctr.bgDark);
  drawTextEx(string("Ver. ") + W_VER, 
             infoSideMargin/2.0f+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2.0f+16, ctr.bgDark);
  
  double copySymWidth = measureTextEx(copySym, copySymSize).x;
  double copyWidth = measureTextEx(copy).x;
  double licenseWidth = measureTextEx(license).x;

  drawTextEx(copySym, infoSideMargin/2.0f+ctr.infoWidth-borderMargin-copySymWidth-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2.0f + 2, ctr.bgDark, 255, copySymSize);
  drawTextEx(copy, infoSideMargin/2.0f+ctr.infoWidth-borderMargin-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2.0f, ctr.bgDark);
  drawTextEx(license, infoSideMargin/2.0f+ctr.infoWidth-borderMargin-licenseWidth, 
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
