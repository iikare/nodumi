#include "dialog.h"
#include "define.h"
#include "data.h"
#include "wrap.h"
#include "aghdef.h"


void dialogController::render() {
  if (preferenceDisplay) { 
    renderPreference();
  }
  if (infoDisplay) {
    renderInfo();
  }
}

void dialogController::renderPreference() {
  const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
  const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
  drawRectangle(prefSideMargin/2, prefTopMargin/2, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);  

  drawTextEx("Preferences", prefSideMargin/2 + 6, prefTopMargin/2 + 6, ctr.bgDark, 255, 18);
}

void dialogController::renderInfo() {
  const int infoSideMargin = ctr.getWidth() - ctr.infoWidth;
  const int infoTopMargin = ctr.getHeight() - ctr.infoHeight;
  drawRectangle(infoSideMargin/2, infoTopMargin/2, ctr.infoWidth, ctr.infoHeight, ctr.bgMenu);  

  Vector2 iconTextVec = measureTextEx(W_NAME, iconTextSize);
  double iconTextHeight = iconTextVec.y;
  double iconBoxWidth = iconTextVec.x + ctr.getImage("ICON").width*iconScale +
                        borderMargin*2 - iconTextAdjust;
  double iconBoxHeight = ctr.getImage("ICON").height*iconScale + borderMargin;
  double iconTextX = infoSideMargin/2 + (ctr.infoWidth - iconBoxWidth)/2 + 
                     ctr.getImage("ICON").width*iconScale + borderMargin - iconTextAdjust;
  double iconTextY = infoTopMargin/2 + ctr.getImage("ICON").height*iconScale/2 + borderMargin - iconTextHeight/2;

  drawTextEx(W_NAME, iconTextX, iconTextY, ctr.bgIcon, 255, iconTextSize);
  Vector2 iconPos = {static_cast<float>(infoSideMargin/2 + (ctr.infoWidth-iconBoxWidth)/2 + borderMargin), 
                     static_cast<float>(infoTopMargin/2 + borderMargin)};
  drawTextureEx(ctr.getImage("ICON"), iconPos, 0, 0.3);
  
  drawTextEx(string("Build Date: ") + BUILD_DATE, 
             infoSideMargin/2+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2, ctr.bgDark);
  drawTextEx(string("Ver. ") + W_VER, 
             infoSideMargin/2+borderMargin, iconPos.y+iconBoxHeight-borderMargin/2+16, ctr.bgDark);
  
  double copySymWidth = measureTextEx(copySym, copySymSize).x;
  double copyWidth = measureTextEx(copy).x;
  double licenseWidth = measureTextEx(license).x;

  drawTextEx(copySym, infoSideMargin/2+ctr.infoWidth-borderMargin-copySymWidth-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2 + 2, ctr.bgDark, 255, copySymSize);
  drawTextEx(copy, infoSideMargin/2+ctr.infoWidth-borderMargin-copyWidth, 
             iconPos.y+iconBoxHeight-borderMargin/2, ctr.bgDark);
  drawTextEx(license, infoSideMargin/2+ctr.infoWidth-borderMargin-licenseWidth, 
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
