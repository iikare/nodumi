#include "sheetctr.h"
#include "define.h"
#include "enum.h"
#include "wrap.h"

void sheetController::drawTimeSignature(pair<int, int> sig, int x, colorRGB col) {

  // y-coordinate is constant in respect to sheet controller parameters, not a user-callable parameter
  const int y = ctr.barMargin+ctr.barWidth*2;
  const int fSize = 157; // constant numeral font size


  // for centering
  //drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});


  auto getGlyphWidth = [&](int codepoint) {
      return GetGlyphInfo(*ctr.getFont("LELAND",fSize), codepoint).image.width;
  };

  if (sig.first < 0 || sig.first > 99 || sig.second < 0 || sig.second > 99) {
    logW(LL_WARN, "complex time signature detected with meter", sig.first, "/", sig.second);
    return;
  }

  // the given x-coordinate is the spatial center of the time signature
  // the given y-coordinate is the top line of the upper staff
  

  auto drawSingleSig = [&](int val, int yOffset) {
    const int point = SYM_TIME_0+val;
    const int halfDist = (getGlyphWidth(point) + 1)/2;

    drawSymbol(point, fSize, x-halfDist, y+yOffset, col);

  };
  auto drawDoubleSig = [&](int val, int yOffset) {
    int padding = 0;
    
    int numL = val/10;
    int numR = val%10;

    if (numL == 1) {
      padding = -2;
    }

    const int pointL = SYM_TIME_0+numL;
    const int widthL= getGlyphWidth(pointL);
    const int pointR = SYM_TIME_0+numR;
    const int widthR= getGlyphWidth(pointR);


    const int leftPos = x - (padding + widthL + widthR)/2;
    const int rightPos = leftPos + widthL + padding;

    drawSymbol(pointL, fSize, leftPos, y+yOffset, col);
    

    drawSymbol(pointR, fSize, rightPos, y+yOffset, col);

  };

  if (sig.first > 9) { // two-digit top
    drawDoubleSig(sig.first, (ctr.sheetHeight-200)/2-ctr.barMargin-ctr.barWidth*2 + 1);
    drawDoubleSig(sig.first, (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }
  else {
    drawSingleSig(sig.first, (ctr.sheetHeight-200)/2-ctr.barMargin-ctr.barWidth*2 + 1);
    drawSingleSig(sig.first, (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }
  
  if (sig.second > 9) { // two-digit bottom
    drawDoubleSig(sig.second, (ctr.sheetHeight-200)/2-ctr.barMargin + 1);
    drawDoubleSig(sig.second, (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing + 1);
  }
  else {
    drawSingleSig(sig.second, (ctr.sheetHeight-200)/2-ctr.barMargin + 1);
    drawSingleSig(sig.second, (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing + 1);
  }

  
}

void sheetController::drawKeySignature(keySig key, int x, colorRGB col) {
  
  const int y = ctr.barMargin+ctr.barWidth*2;

  
  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});

  int symbol = SYM_NONE;
  int prevAcc = 0;
  int prevType = SYM_ACC_NATURAL;

  if (key.getAcc() == 0) {
    // C major / A minor
    // find number of courtesy accidentals
    symbol = SYM_ACC_NATURAL;
    if (key.getPrev() != nullptr) {
      prevAcc = key.getPrev()->getSize();
      logQ("prevAcc", prevAcc);
    }
    if (prevAcc != 0) {
      prevType = key.getPrev()->isSharp() ? SYM_ACC_SHARP : SYM_ACC_FLAT;
    }
  }
  else if (key.getAcc() > 0) {
    symbol = SYM_ACC_SHARP;
  }
  else {
    symbol = SYM_ACC_FLAT;
  }



  const vector<int> sharpHash    = {1, 4, 0, 3, 6, 2, 5};
  const vector<int> sharpSpacing = {0, 1, 0, 2, 4, 4, 5};
  const vector<int> sharpY       = {-1, -1, -2, -2, 0, -1, -1};

  const vector<int> flatHash     = {5, 2, 6, 3, 7, 4, 8};
  const vector<int> flatSpacing  = {0, 0, 0, 1, 1, 2, 2};
  const vector<int> flatY        = {0, -1, 0, -2, 0, -1, 0};

  
  // unimplemented
  const vector<int> naturalHashS = {0,0,0,0,0,0,0};
  const vector<int> naturalHashF = {0,0,0,0,0,0,0};
 
  auto drawKeySigPart = [&] (int symbol, int index, int prevType) {

    if (index > (int)sharpHash.size() || index < 0) { return; }

    vector<int>& accHash = const_cast<vector<int>&>(sharpHash);
    vector<int>& accSpacing = const_cast<vector<int>&>(sharpSpacing);
    vector<int>& accY = const_cast<vector<int>&>(sharpY);
    float posMod = accHash[index];
    
    switch(symbol) {
      case SYM_ACC_SHARP:
        accHash = sharpHash;
        accSpacing = sharpSpacing;
        accY = sharpY;
        posMod = accHash[index]-1;
        break;
      case SYM_ACC_FLAT:
        accHash = flatHash;
        accSpacing = flatSpacing;
        accY = flatY;
        posMod = accHash[index]-1;
        break;
      case SYM_ACC_NATURAL:
        logQ("NATURAL NOT IMPLEMENTED YET");
        if (prevType == SYM_ACC_SHARP) {
          accHash = sharpHash;
          accSpacing = sharpSpacing;
          accY = sharpY;
          posMod = accHash[index]-1;
        }
        else {
          accHash = flatHash;
          accSpacing = flatSpacing;
          accY = flatY;
          posMod = accHash[index]-1;
        }
        break;

    }
    //posMod = index;
   //symbol = SYM_ACC_NATURAL; 
    const int accConstSpacing = 10;

    if (symbol == SYM_ACC_NATURAL) logQ("zoom");

    drawSymbol(symbol, fSize, x+index*accConstSpacing+accSpacing[index], 
                              y-ctr.barSpacing+posMod*(ctr.barWidth/2.0f+0.4)-accY[index], col);
    drawSymbol(symbol, fSize, x+index*accConstSpacing+accSpacing[index], 
                              y+posMod*(ctr.barWidth/2.0f+0.4)-accY[index]+1+ctr.barWidth, col);
  };

  int drawLimit = symbol == SYM_ACC_NATURAL ? prevAcc : key.getSize();

  for (auto i = 0; i < drawLimit; i++) {
  
    drawKeySigPart(symbol, i, prevType);

  }

}


void sheetController::drawNote(sheetNote noteData, int x, colorRGB col) {
  
  const int y = ctr.barMargin+ctr.barWidth*2;

  
  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});


  drawSymbol(SYM_HEAD_STD, fSize, x, y+1, col);
    
}

int sheetController::getKeyWidth(keySig key) {

  return key.getKey();
}
