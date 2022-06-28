#include "sheetctr.h"
#include "define.h"
#include "enum.h"
#include "wrap.h"

void sheetController::addTimeSignature(int position, int tick, timeSig timeSignature) {
  if (timeSignatureMap.size() != 0 && timeSignatureMap[timeSignatureMap.size()-1].second == timeSignature) {
    return;
  }
  timeSignature.setTick(tick);
  timeSignatureMap.push_back(make_pair(position, timeSignature));
}

void sheetController::addKeySignature(int position, int tick, keySig keySignature) {
  if (keySignatureMap.size() != 0 && keySignatureMap[keySignatureMap.size()-1].second == keySignature) {
    return;
  }
  keySignature.setTick(tick);
  keySignatureMap.push_back(make_pair(position, keySignature));
}

keySig sheetController::eventToKeySignature(int keySigType, bool isMinor) {
  int keyType = KEYSIG_NONE;

  switch(keySigType) {
    case 0:
      keyType = KEYSIG_C;
      break;
    case 1:
      keyType = KEYSIG_G;
      break;
    case 2:
      keyType = KEYSIG_D;
      break;
    case 3:
      keyType = KEYSIG_A;
      break;
    case 4:
      keyType = KEYSIG_E;
      break;
    case 5:
      keyType = KEYSIG_B;
      break;
    case 6:
      keyType = KEYSIG_FSHARP;
      break;
    case 7:
      keyType = KEYSIG_CSHARP;
      break;
    case 255:
      keyType = KEYSIG_F;
      break;
    case 254:
      keyType = KEYSIG_BFLAT;
      break;
    case 253:
      keyType = KEYSIG_EFLAT;
      break;
    case 252:
      keyType = KEYSIG_AFLAT;
      break;
    case 251:
      keyType = KEYSIG_DFLAT;
      break;
    case 250:
      keyType = KEYSIG_GFLAT;
      break;
    case 249:
      keyType = KEYSIG_CFLAT;
      break;
  }


  keySig finalKey = keySig(keyType, isMinor, -1);

  return finalKey;
}

timeSig sheetController::getTimeSignature(int offset) {
  timeSig timeSignature = {0, -1, 1};
  for (unsigned int i = 0; i < timeSignatureMap.size(); i++) {
    if (offset > timeSignatureMap[i].first && offset < timeSignatureMap[i + 1].first) {
      return timeSignatureMap[i].second;
    }
  }
  return timeSignature;
}

keySig sheetController::getKeySignature(int offset) {
  keySig keySignature = {0, 1, -1};
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (offset > keySignatureMap[i].first && offset < keySignatureMap[i + 1].first) {
      return keySignatureMap[i].second;
    }
  }
  return keySignature;
}

void sheetController::reset() {
  timeSignatureMap.clear();
  keySignatureMap.clear();
}

void sheetController::linkKeySignatures() {
  if (keySignatureMap.size() == 0) {
    return;
  }
  for (unsigned int i = 0; i < keySignatureMap.size(); i++) {
    if (i == 0) {
      continue;
    }
    keySignatureMap[i].second.setPrev(&keySignatureMap[i].second);
  }
}

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
    drawDoubleSig(sig.first, -ctr.barMargin-ctr.barWidth*2 + 1);
    drawDoubleSig(sig.first, -ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }
  else {
    drawSingleSig(sig.first, -ctr.barMargin-ctr.barWidth*2 + 1);
    drawSingleSig(sig.first, -ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }
  
  if (sig.second > 9) { // two-digit bottom
    drawDoubleSig(sig.second, -ctr.barMargin + 1);
    drawDoubleSig(sig.second, -ctr.barMargin+ctr.barSpacing + 1);
  }
  else {
    drawSingleSig(sig.second, -ctr.barMargin + 1);
    drawSingleSig(sig.second, -ctr.barMargin+ctr.barSpacing + 1);
  }

  
}

void sheetController::drawKeySignature(keySig key, int x, colorRGB col) {
  
  const int y = ctr.barMargin+ctr.barWidth*2;
  const int fSize = 157; // constant numeral font size
  
  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});

  int symbol = SYM_NONE;

  if (key.getAcc() == 0) {
    // C major / A minor
    // find number of courtesy accidentals
    symbol = SYM_ACC_NATURAL;
  }
  else if (key.getAcc() > 0) {
    symbol = SYM_ACC_SHARP;
  }
  else {
    symbol = SYM_ACC_FLAT;
  }



  const vector<int> sharpHash    = {1, 4, 0, 3, 6, 2, 5};
  const vector<int> sharpSpacing = {0, 1, 0, 2, 4, 4, 5};
  const vector<int> sharpY       = {-2, -1, -2, -2, 0, -1, -1};

  const vector<int> flatHash     = {5, 2, 6, 3, 7, 4, 8};
  const vector<int> flatSpacing  = {0, 0, 0, 1, 1, 2, 2};
  const vector<int> flatY        = {0, 0, 0, 0, 0, -1, 0};

  
  // unimplemented
  const vector<int> naturalHashS = {0,0,0,0,0,0,0};
  const vector<int> naturalHashF = {0,0,0,0,0,0,0};
 
  auto drawKeySigPart = [&] (int symbol, int index) {

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
        accHash = sharpHash;
        break;

    }
    //posMod = index;
    
    const int accConstSpacing = 10;


    drawSymbol(symbol, fSize, x+index*accConstSpacing+accSpacing[index], 
                              y-ctr.barSpacing+posMod*(ctr.barWidth/2.0f+0.4)-accY[index], col);
    drawSymbol(symbol, fSize, x+index*accConstSpacing+accSpacing[index], 
                              y+posMod*(ctr.barWidth/2.0f+0.4)-accY[index]+1, col);
  };

  for (auto i = 0; i < abs(key.getAcc()); i++) {
  
    drawKeySigPart(symbol, i);

  }

}


void sheetController::drawNote(sheetNote noteData, int x, colorRGB col) {
  
  const int y = ctr.barMargin+ctr.barWidth*2;
  const int fSize = 157; // constant numeral font size
  
  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});


  drawSymbol(SYM_DYN_MF, fSize, x, y, col);
    
}
