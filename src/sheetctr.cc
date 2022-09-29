#include <numeric>
#include "sheetctr.h"
#include "define.h"
#include "enum.h"
#include "wrap.h"

using std::accumulate;

int sheetController::getGlyphWidth(int codepoint, int size) {
  return GetGlyphInfo(*ctr.getFont("LELAND", size), codepoint).image.width;
}

void sheetController::drawTimeSignature(const timeSig& time, int x) {
  // y-coordinate is constant in respect to sheet controller parameters, not a user-callable parameter
  const int y = ctr.barMargin+ctr.barWidth*2;
  const colorRGB col = ctr.bgSheetNote;


  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});

  if (time.getTop() < 0 || time.getTop() > 99 || time.getBottom() < 0 || time.getBottom() > 99) {
    logW(LL_WARN, "complex time signature detected with meter", time.getTop(), "/", time.getBottom());
    return;
  }

  x = x + getTimeWidth(timeSig(time.getTop(),time.getBottom(),0))/2;

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

  if (time.getTop() > 9) { // two-digit top
    drawDoubleSig(time.getTop(), (ctr.sheetHeight-200)/2-ctr.barMargin-ctr.barWidth*2 + 1);
    drawDoubleSig(time.getTop(), (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }
  else {
    drawSingleSig(time.getTop(), (ctr.sheetHeight-200)/2-ctr.barMargin-ctr.barWidth*2 + 1);
    drawSingleSig(time.getTop(), (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing-ctr.barWidth*2 + 1);
  }

  if (time.getBottom() > 9) { // two-digit bottom
    drawDoubleSig(time.getBottom(), (ctr.sheetHeight-200)/2-ctr.barMargin + 1);
    drawDoubleSig(time.getBottom(), (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing + 1);
  }
  else {
    drawSingleSig(time.getBottom(), (ctr.sheetHeight-200)/2-ctr.barMargin + 1);
    drawSingleSig(time.getBottom(), (ctr.sheetHeight-200)/2-ctr.barMargin+ctr.barSpacing + 1);
  }
}

void sheetController::drawKeySignature(const keySig& key, int x) {
  const int y = ctr.barMargin+ctr.barWidth*2;
  const colorRGB col = ctr.bgSheetNote;

  // for centering
  drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});

  int symbol = SYM_NONE;
  int prevAcc = 0;
  int prevType = SYM_ACC_NATURAL;
  findKeyData(key, symbol, prevAcc, prevType);
  
 
  auto drawKeySigPart = [&] (const int symbol, const int index, const int prevType) {
    if (index > accMax || index < 0) { return; }
    
    int* accHash = const_cast<int*>(flatHash);
    int* accSpacing = const_cast<int*>(flatSpacing);
    int* accY = const_cast<int*>(flatY);

    double posMod = 0;
    
    switch(symbol) {
      case SYM_ACC_SHARP:
        accHash = const_cast<int*>(sharpHash);
        accSpacing = const_cast<int*>(sharpSpacing);
        accY = const_cast<int*>(sharpY);
        posMod = accHash[index]-1;
        break;
      case SYM_ACC_FLAT:
        accHash = const_cast<int*>(flatHash);
        accSpacing = const_cast<int*>(flatSpacing);
        accY = const_cast<int*>(flatY);
        posMod = accHash[index]-1;
        break;
      case SYM_ACC_NATURAL:
        switch(prevType) {
          case SYM_ACC_SHARP:
            accHash = const_cast<int*>(sharpHash);
            accSpacing = const_cast<int*>(sharpSpacing);
            accY = const_cast<int*>(sharpY);
            posMod = accHash[index]-1;
            break;
          case SYM_ACC_FLAT:
            accHash = const_cast<int*>(flatHash);
            accSpacing = const_cast<int*>(flatSpacing);
            accY = const_cast<int*>(flatY);
            posMod = accHash[index]-1;
            break;
        }
        break;
    }
    //logQ(getGlyphWidth(symbol) + index*accConstSpacing+accSpacing[index]);

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


void sheetController::drawNote(const sheetNote& noteData, int x, colorRGB col) {
  const int y = findStaveY(noteData.oriNote->sheetY, noteData.stave);;
//retun;
  // for centering
  //drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});
  //drawLineEx(x, 0,
             //x, ctr.getHeight(), 1, ctr.bgNow);

  drawLineEx(x, y-1, x, y+30, stemWidth, ctr.bgSheetNote);
  int sym = SYM_HEAD_STD;
  drawSymbol(sym, fSize, x+0*getSymbolWidth(sym)-stemWidth/2, y+2-ctr.barSpacing, col);
  drawSymbol(SYM_FLAG_8D, fSize, x-+stemWidth/2, y+30+5-ctr.barSpacing, col);
}

int sheetController::findStaveY(int sheetY, int stave) {
  if (stave == STAVE_TREBLE) {
    return ctr.barMargin+ctr.barWidth*7-ctr.barWidth/2*sheetY;
  }
  else {
    return ctr.barMargin+ctr.barSpacing+ctr.barWidth-ctr.barWidth/2*sheetY;
  }
}

int sheetController::getKeyWidth(const keySig& key) {
  int symbol = SYM_NONE;
  int prevAcc = 0;
  int prevType = SYM_ACC_NATURAL;
  findKeyData(key, symbol, prevAcc, prevType);
  
  //drawLine(x,0,x,ctr.getHeight(),ctr.bgNow);
 
  int xBound = 0;
  switch(symbol) {
    case SYM_ACC_SHARP:
      xBound += sharpWidths[key.getSize()];
      break;
    case SYM_ACC_FLAT:
      xBound += flatWidths[key.getSize()];
      break;
    case SYM_ACC_NATURAL:
        // adjustments are due to difference in width between the accidental symbols
        switch(prevType) {
          case SYM_ACC_SHARP:
            xBound += sharpWidths[prevAcc]-3;
            break;
          case SYM_ACC_FLAT:
            xBound += flatWidths[prevAcc]-1;
            break;
        }
      break;
  }

  //drawLine(x+xBound,0,x+xBound,ctr.getHeight(),ctr.bgSheetNote);

  return xBound;
}

int sheetController::getTimeWidth(const timeSig& key) {
  return max(findTimePartWidth(key.getTop()), findTimePartWidth(key.getBottom()));
}

int sheetController::findTimePartWidth(const int part) {
  // error checking already done earlier
  if (part < 10) {
    return timeWidths[part]; 
  }

  // account for shortening of "1" glyph in range 10-19
  if (part/10 == 1) {
    return timeWidths[part/10]-2+timeWidths[part%10];
  }
  
  return timeWidths[part/10]+timeWidths[part%10];
}

void sheetController::findKeyData(const keySig& key, int& symbol, int& prevAcc, int& prevType) {
  symbol = SYM_NONE;
  prevAcc = 0;
  prevType = SYM_NONE;

  if (key.getAcc() == 0) {
    // C major / A minor
    // find number of courtesy accidentals
    symbol = SYM_ACC_NATURAL;
    if (key.getPrev() != nullptr) {
      prevAcc = key.getPrev()->getSize();
    }
    if (prevAcc != 0) {
      prevType = key.getPrev()->isSharp() ? SYM_ACC_SHARP : SYM_ACC_FLAT;
    }
  }
  else if (key.isSharp()) {
    symbol = SYM_ACC_SHARP;
  }
  else {
    symbol = SYM_ACC_FLAT;
  }
}

void sheetController::disectMeasure(measureController& measure) {
  //logQ("measure", measure.getNumber(), "has timesig", measure.currentTime.getTop(), measure.currentTime.getBottom());
  // preprocessing
  sheetMeasure dm;
  dm.setParent(measure);
  dm.buildChordMap(measure.displayNotes);

  // run a DFA for each valid midi key, advancing state based on new-note-to-midi-position values
  // each DFA set is unique to its measure
  // add "1" to size for inclusive bounding
  vector<int> presentDFAState;
  findDFAStartVector(presentDFAState, measure.currentKey); 

  // find true accidental type for each note
  for (unsigned int ch = 0; const auto& c : dm.chords) {
    for (unsigned int ct = 0; const auto& n : c.second) {
      if (n->oriNote->sheetY > getStaveRenderLimit().first || n->oriNote->sheetY < getStaveRenderLimit().second) {
        continue;
      }
      //int& keyPos = presentDFAState[n->oriNote->sheetY];
      n->displayAcc = getDisplayAccType(presentDFAState[mapSheetY(n->oriNote->sheetY)], n->oriNote->accType);
      //logQ("pos:", n->oriNote->sheetY,presentDFAState[mapSheetY(n->oriNote->sheetY)], n->displayAcc, n->oriNote->accType);
      
      if (n->oriNote->sheetY > 0) {
        n->stave = STAVE_TREBLE;
      }
      else if (n->oriNote->sheetY < 0) {
        n->stave = STAVE_BASS;
      }
      else {
        //n->stave = STAVE_TREBLE;
        ct >= c.second.size()/2 ? n->stave = STAVE_TREBLE : (ct != 0 ? n->stave = STAVE_BASS : n->stave = STAVE_TREBLE);
      }
      ct++;
    }
    ch++;
  }

  // remove duplicate notes (rendering on same position)
  for (unsigned int ch = 0; const auto& c : dm.chords) {
    for (unsigned int ct = 0; const auto& n : c.second) {
      if (ct > 0 && n->oriNote->sheetY - dm.chords[ch].second[ct-1]->oriNote->sheetY == 0) {
          //n->displayAcc == dm.chords[ch].second[ct-1]->displayAcc) {
        n->visible = false;
      }

      // hide out-of-bounds notes
      if ((n->oriNote->sheetY > getStaveRenderLimit().first || 
          n->oriNote->sheetY < getStaveRenderLimit().second)) {
        n->visible = false;
      }
      ct++;
    }
    ch++;
  }

  dm.buildFlagMap();


  // TODO: redo left-detection using flagmap info
  
  for (unsigned int ch = 0; const auto& c : dm.chords) {
    int stavect = 0;
    for (unsigned int ct = 0; const auto& n : c.second) {
      if (n->oriNote->sheetY > getStaveRenderLimit().first || n->oriNote->sheetY < getStaveRenderLimit().second) {
        continue;
      }
      if (!n->visible) {
        continue;
      }

      if (ct != 0) {
        if (n->stave != dm.chords[ch].second[ct-1]->stave) {
          stavect = 0;
        }
      }
      if (stavect != 0) {
        if (n->oriNote->sheetY - dm.chords[ch].second[ct-1]->oriNote->sheetY < 2) {
          n->left = !dm.chords[ch].second[ct-1]->left;
        }
      }
      ct++;
      stavect++;
    }
    ch++;
  }
 
  // TODO: incorporate flag widths into chord sizing detection

  //calculate chord sizes
  for (unsigned int ch = 0; const auto& c : dm.chords) {
    //logQ("x");

    int leftWidth = 0;
    int rightWidth = 0;

    int chordStemWidth = dm.hasStem(ch) ? stemWidth : 0;


    for (const auto& n : c.second) {
      int noteLW = 0;
      int noteRW = 0;
      if (!n->visible) {
        continue;
      }
      if (n->oriNote->sheetY > getStaveRenderLimit().first || n->oriNote->sheetY < getStaveRenderLimit().second) {
        noteLW = placeholderWidth/2;
        noteRW = placeholderWidth/2;
      }
      else {
        if (n->left) {
          noteLW += getSymbolWidth(getSymbolType(n->oriNote->type))+chordStemWidth/2;
        }
        else {
          noteRW += getSymbolWidth(getSymbolType(n->oriNote->type))+chordStemWidth/2;

        }

        if (n->oriNote->hasDot()) {
          noteRW += getSymbolWidth(SYM_DOT) + dotSpacing;
        }

        if (n->displayAcc != ACC_NONE) {
          noteLW += getSymbolWidth(getSymbolType(n->displayAcc)) + accSpacing;
        }
      }
      leftWidth = max(leftWidth, noteLW);
      rightWidth = max(rightWidth, noteRW);
    }

    // get flag widths
   for (const auto& fStem : dm.chordData[ch].flags) {

     int flagWidth = getSymbolWidth(fStem.flagType, fStem.flagDir);
    
     // in both directions, the flag width is added to right-of-stem
     rightWidth = max(rightWidth, flagWidth);
   }


    dm.chordData[ch].leftWidth = leftWidth;
    dm.chordData[ch].rightWidth = rightWidth;

    ch++;
  }

  //logQ("MEASURE", measure.getNumber(), "with length", measure.getTickLen());



  if (measure.notes.size() == 0) {
    // TODO: fill space with rests

  }

  displayMeasure.push_back(dm);
}

int sheetController::findMeasureWidth(int measureNum, bool includeSig) {
  sheetMeasure& dm = displayMeasure[measureNum];

  int width = sigSpacing*2; // default side spacing

  for (const auto& ts : dm.measure->timeSignatures) {
    width += getTimeWidth(ts);
    width += sigSpacing;
  }
  for (const auto& ks : dm.measure->keySignatures) {
    width += getKeyWidth(ks);
    width += sigSpacing;
  }

  if (dm.measure->keySignatures.empty() && includeSig) {
    width += getKeyWidth(dm.measure->currentKey);
    width += sigSpacing;
  }

  //int tickDist = dm.chords.size() < 2 ? 0 : __INT_MAX__;

  for (unsigned int c = 0; c < dm.chords.size(); ++c) {
    //if (c != 0) {
      //tickDist = min(tickDist, i.first - dm.chords[c-1].first);
    //}

    width += dm.chordData[c].getSize();
  }

  //logQ("m", measureNum+1, "min dist.", tickDist, "width", width, "chords");

  return width;
}

void sheetController::findSheetPages() {
  sheetPageSeparator.clear();
  sheetPageSeparator.push_back(0);

  int pageWidth = 0;
  int measureWidth = 0;
  int maxWidth = ctr.getWidth() - ctr.sheetSideMargin - ctr.sheetSymbolWidth;

  for (int i = 0; i < ctr.getMeasureCount(); ++i) { 
    measureWidth = findMeasureWidth(i);
    //logQ(i+1, measureWidth);
    if (pageWidth + measureWidth > maxWidth) {
      //logQ("divider at", i+1, "extra space", maxWidth - pageWidth);
      sheetPageSeparator.push_back(i);
      pageWidth = 0;
      measureWidth = findMeasureWidth(i, true); // include current keysig
    }
    pageWidth += measureWidth;
  }
}

pair<int, int> sheetController::findSheetPageLimit(int measureNum) const {
  // return value is inclusive in interval [min, max]
  pair<int, int> result = { 0, 0 };

  if (sheetPageSeparator.empty()) {
    return result;
  }

  if (measureNum > sheetPageSeparator[sheetPageSeparator.size()-1]) {
    result.first = sheetPageSeparator[sheetPageSeparator.size()-1] + 1; // is 0-indexed
    result.second = ctr.getMeasureCount();            // is 1-indexed
  }
  else {
    for (unsigned int i = 0; i < sheetPageSeparator.size() - 1; ++i) {
      if (measureNum > sheetPageSeparator[i]) {
        result.first = sheetPageSeparator[i] + 1;
        result.second = sheetPageSeparator[i + 1];
      }
    }
  }

  return result;
}

void sheetController::findDFAStartVector(vector<int>& DFAState, const keySig& ks) {
  DFAState.resize(MAX_STAVE_IDX - MIN_STAVE_IDX + 1);


  const int* keyStateTemplate = staveKeySigMap[ks.getKey()];

  for (unsigned int idx = 0; idx < DFAState.size(); ++idx) {
    int templateIdx = idx + abs(MIN_STAVE_IDX) + (abs(MIN_STAVE_IDX) % 7);
    // mark middle C
    //int state = idx == -MIN_STAVE_IDX ? -1 : keyStateTemplate[(1 + templateIdx) % 7]; 
    int state = keyStateTemplate[(1 + templateIdx) % 7]; 
    DFAState[idx] = state;
  }
}

int sheetController::mapSheetY(int sheetY) {
  // input [0, MIN_STAVE_IDX + MAX_STAVE_IDX + 1]
  return sheetY + abs(MIN_STAVE_IDX);
}

void sheetController::drawSheetPage() {
  int offset = ctr.sheetSymbolWidth;

  pair<int, int> measureRange = findSheetPageLimit(ctr.getCurrentMeasure());

  int spacingPositions = 0;
  int margin = ctr.getWidth() - ctr.sheetSideMargin - ctr.sheetSymbolWidth;

  for (int m = measureRange.first; m <= measureRange.second; ++m) {
    margin -= findMeasureWidth(m-1, m == measureRange.first);
    spacingPositions += displayMeasure[m-1].getSpacingCount();
  }

  vector<int> spacingMargin (abs(margin) % spacingPositions, margin / spacingPositions + (margin > 0 ? 1 : -1));
  vector<int> spacingExtra (spacingPositions - (abs(margin) % spacingPositions), margin / spacingPositions);
  spacingMargin.insert(spacingMargin.end(), spacingExtra.begin(), spacingExtra.end());

  int spacingIndex = 0;
  for (int m = measureRange.first; m <= measureRange.second; ++m) {
    //logQ(margin, spacingPositions, margin/spacingPositions, margin % spacingPositions, 
         //std::accumulate(spacingMargin.begin(), spacingMargin.end(), 0));


    // beginning of measure spacing (not first measure)
    if (m != measureRange.first) {
      offset += sigSpacing;
    }

    if (m == measureRange.first && displayMeasure[m-1].measure->keySignatures.empty()) {
      drawKeySignature(displayMeasure[m-1].measure->currentKey, offset); 
      offset += getKeyWidth(displayMeasure[m-1].measure->currentKey);
    }
    else { 
      for (const auto& ks : displayMeasure[m-1].measure->keySignatures) {
        drawKeySignature(ks, offset); 
        offset += getKeyWidth(ks) + sigSpacing;
      }
      for (const auto& ts : displayMeasure[m-1].measure->timeSignatures) {
        drawTimeSignature(ts, offset); 
        offset += getTimeWidth(ts) + sigSpacing;
      }
    }
    
    drawTextEx(to_string(m), {(float)offset+sigSpacing/2 , ctr.menuHeight + ctr.barMargin - ctr.barWidth*1.75}, ctr.bgSheetNote);

    for (unsigned int ch = 0; ch < displayMeasure[m-1].chords.size(); ++ch) {
      
      int chordSize = displayMeasure[m-1].chordData[ch].getSize();

      // spacing calculated only between chords
      if (ch != 0) {
        offset += spacingMargin[spacingIndex++];
      }
      
      // TODO: draw chords WITH spacing

      int stemPos = offset + displayMeasure[m-1].chordData[ch].getStemPosition();

      for (unsigned int n = 0; n < displayMeasure[m-1].chords[ch].second.size(); ++n) {


        sheetNote* note = displayMeasure[m-1].chords[ch].second[n];

        if (!note->visible) {
          continue;
        }

        const int y = findStaveY(note->oriNote->sheetY, note->stave);;
        // for centering
        //drawRing({static_cast<float>(x), static_cast<float>(y)}, 0, 2, {255,0,0});
        //drawLineEx(x, 0,
                   //x, ctr.getHeight(), 1, ctr.bgNow);

        int noteSym = getSymbolType(note->oriNote->type);
        int noteX = stemPos - stemWidth/2;
        if (note->left) {
          noteX = stemPos - getSymbolWidth(noteSym) + stemWidth/2;
          
        }



        //drawLineEx(noteX, y-1, noteX, y+30, stemWidth, ctr.bgSheetNote);
        drawSymbol(noteSym, fSize, noteX+0*getSymbolWidth(noteSym)-stemWidth/2, y+2-ctr.barSpacing, ctr.bgSheetNote);
        //drawSymbol(SYM_FLAG_8D, fSize, noteX-+stemWidth/2, y+30+5-ctr.barSpacing, ctr.bgSheetNote);
      }
      


      //logQ(displayMeasure[m-1].chordData[ch].flags.size());
      for (int stc = 0; const auto& stem : displayMeasure[m-1].chordData[ch].flags) {
        int stave = STAVE_TREBLE;//stc++ % 2 ? STAVE_BASS : STAVE_TREBLE; // hack for two flag chords
        int lOffset = displayMeasure[m-1].chordData[ch].getStemPosition();
        drawLineEx(offset+lOffset, findStaveY(stem.startY, stem.stave),
                   offset+lOffset, findStaveY(stem.endY, stem.stave), 2, ctr.bgSheetNote);
      }
      drawLineEx(offset, ctr.menuHeight + ctr.barMargin,
                 offset, ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgNow);

      offset += chordSize;


    }

    // end of measure spacing
    offset += sigSpacing;

    // end of measure bars (except the end)
    if (m != measureRange.second) {
      drawLineEx(offset, ctr.menuHeight + ctr.barMargin,
                 offset, ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
    }


  }
  //logQ(formatVector(spacingMargin));

}

int sheetController::getDisplayAccType(int& DFAState, int noteAccType) {
  switch(DFAState) {
    case DA_STATE_CLEAR:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_CLEAR;
          return ACC_NONE;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_SHARP:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP_MULT;
          return ACC_NONE;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_FLAT:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT_MULT;
          return ACC_NONE;
      }
      break;
    case DA_STATE_NATURAL:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_CLEAR;
          return ACC_NONE;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_SHARP_MULT:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP_MULT;
          return ACC_NONE;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT;
          return ACC_FLAT;
      }
      break;
    case DA_STATE_FLAT_MULT:
      switch(noteAccType) {
        case ACC_NONE:
          DFAState = DA_STATE_NATURAL;
          return ACC_NATURAL;
        case ACC_SHARP:
          DFAState = DA_STATE_SHARP;
          return ACC_SHARP;
        case ACC_FLAT:
          DFAState = DA_STATE_FLAT_MULT;
          return ACC_NONE;
      }
      break;
    default:
      logW(LL_WARN, "invalid DFA state or accidental type for",
                    "display accidental calculation (state, type):", DFAState, noteAccType);
      break;
  }

  return -1;
}

int sheetController::getSymbolWidth(const int symbol) {

  switch(symbol) {
    case SYM_HEAD_WHOLE:
      return 15;
    case SYM_HEAD_HALF:
      return 13;
    case SYM_HEAD_STD:
      return 13;
    case SYM_FLAG_8U:
      return 12;
    case SYM_FLAG_8D:
      return 13;
    case SYM_FLAG_16U:
      return 11;
    case SYM_FLAG_16D:
      return 13;
    case SYM_FLAG_32U:
      return 11;
    case SYM_FLAG_32D:
      return 13;
    case SYM_FLAG_64U:
      return 11;
    case SYM_FLAG_64D:
      return 13;
    case SYM_ACC_FLAT:
      return 8;
    case SYM_ACC_NATURAL:
      return 7;
    case SYM_ACC_SHARP:
      return 10;
    case SYM_DOT:
      return 4;
    default:
      logQ("invalid symbol:", symbol);
      break;
  }

  return -1;
}

int sheetController::getSymbolWidth(const int flagType, const int dir) {
  switch (flagType) {
    case FLAGTYPE_STEM:
      return stemWidth;
    case FLAGTYPE_8:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_8U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_8D);
      }
      break;
    case FLAGTYPE_16:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_16U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_16D);
      }
      break;
    case FLAGTYPE_32:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_32U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_32D);
      }
      break;
    case FLAGTYPE_64:
      switch (dir) {
        case FLAG_UP:
          return getSymbolWidth(SYM_FLAG_64U);
        case FLAG_DOWN:
          return getSymbolWidth(SYM_FLAG_64D);
      }
      break;
    case FLAGTYPE_NONE:
      return 0;
  }

  return -1;
}

int sheetController::getSymbolType(const int noteType) {
  switch(noteType) {
    case NOTE_LARGE:
    case NOTE_WHOLE_DOT:
    case NOTE_WHOLE:
      return SYM_HEAD_WHOLE;
    case NOTE_HALF_DOT:
    case NOTE_HALF:
      return SYM_HEAD_HALF;
    case NOTE_QUARTER_DOT:
    case NOTE_QUARTER:
    case NOTE_8_DOT:
    case NOTE_8:
    case NOTE_16_DOT:
    case NOTE_16:
    case NOTE_32:
    case NOTE_64:
      return SYM_HEAD_STD;
    default:
      logQ("invalid note type:", noteType);
      break;
  }
  
  return -1;
}

