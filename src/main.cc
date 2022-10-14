#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
  #include "../dpd/raylib/src/rlgl.h"
#else
  #include <raylib.h>
  #include <rlgl.h>
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <algorithm>
#include <bit>
#include "../dpd/osdialog/osdialog.h"
#include "aghfile.h"
#include "enum.h"
#include "box.h"
#include "log.h"
#include "misc.h"
#include "menu.h"
#include "data.h"
#include "wrap.h"
#include "lerp.h"
#include "image.h"
#include "color.h"
#include "define.h"
#include "menuctr.h"
#include "controller.h"

using std::string;
using std::vector;
using std::unordered_map;
using std::min;
using std::max;
using std::to_string;
using std::thread;
using std::ref;
using std::__countl_zero;

controller ctr;

int main (int argc, char* argv[]) {

  // basic window setup
  SetTraceLogLevel(LOG_WARNING);

  // debug
  SetTraceLogLevel(LOG_INFO);
  
  string windowTitle = string(W_NAME) + " " + string(W_VER);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(W_WIDTH, W_HEIGHT, windowTitle.c_str());
  SetTargetFPS(60);
  SetExitKey(KEY_F7);
  SetWindowMinSize(W_WIDTH, W_HEIGHT);

  
  // no packed executable data can be loaded before this point
  ctr.init(assetSet);

  
 
  // program-wide variables 

  // scaling settings
  double zoomLevel = 0.125;
  double timeOffset = 0;
  const double shiftC = 2.5;
  double pauseOffset = 0;

  // play settings
  bool run = false;

  // view settings
  bool nowLine = true;
  bool showFPS = false;
  bool showImage = true;
  bool colorMove = false;
  bool colorSquare = false;
  bool colorCircle = false;
  bool sheetMusicDisplay = false;
  bool preferenceDisplay = false;
  bool infoDisplay = false;
 
  bool measureLine = true;
  bool measureNumber = true;

  int songTimeType = SONGTIME_NONE;
  int tonicOffset = 0;
  int displayMode = DISPLAY_FFT;
  
  double nowLineX = ctr.getWidth()/2.0f;

  string FPSText = "";

  enumChecker<hoverType> hoverType;

  // color variables
  int selectType = SELECT_NONE;
  int colorMode = COLOR_PART;
  vector<colorRGB>* colorSetOn = &ctr.setTrackOn;
  vector<colorRGB>* colorSetOff = &ctr.setTrackOff;

  // file IO controllers
  bool newFile = false;
  char* filenameC = nullptr;
  string filename = "";
  osdialog_filters* filetypes = osdialog_filters_parse("midi/mki:mid,mki");
  
  bool newImage = false;
  char* imagenameC = nullptr;
  string imagename = "";
  osdialog_filters* imagetypes = osdialog_filters_parse("image:png,jpeg,jpg");
  
  char* savenameC = nullptr;
  string savename = "";
  osdialog_filters* savetypes = osdialog_filters_parse("mki:mki");

  fileType curFileType = FILE_NONE; 

  // data selector
  midi& noteData = ctr.liveInput.noteStream;

  // right click variables
  int clickNote = -1;
  int clickTmp = -1;
  bool clickOn = false;
  bool clickOnTmp = false;

  // menu variables
  Vector2 songTimePosition = {6.0f, 26.0f};

  // screen space conversion functions
  const auto convertSSX = [&] (int value) {
    return nowLineX + (value - timeOffset) * zoomLevel;
  };

  const auto convertSSY = [&] (int value) {
    return (ctr.getHeight() - (ctr.getHeight() - (ctr.menuHeight + ctr.barHeight)) *
            static_cast<double>(value - MIN_NOTE_IDX + 3) / (NOTE_RANGE + 4));
  };

  // inverse screen space conversion functions
  const auto unconvertSSX = [&] (int value) {
    return timeOffset + (value-nowLineX)/zoomLevel;
  };

  const auto inverseSSX = [&] () {

    //double spaceR = ctr.getWidth() - nowLineX;

    double minVal = max(0.0, timeOffset - nowLineX/zoomLevel);

    // extra "1" prevents roundoff error
    double maxVal = min(ctr.getLastTime(), static_cast<int>(timeOffset+(ctr.getWidth()+1-nowLineX)/zoomLevel));


    return make_pair(minVal, maxVal);
  };
  
  // dialog box hover detection
  const auto hoverDialog = [&](bool diaDisplay, int x, int y, int w, int h) {
    if (diaDisplay) {
      const rect boundingBox = { x, y, w, h };
      if(pointInBox(ctr.getMousePosition(), boundingBox)) {
        hoverType.add(HOVER_DIALOG);
      }
    }
  };


  // menu objects
  vector<string> fileMenuContents = {"File", "Open File", "Open Image", "Save", "Save As", "Exit"};
  menu fileMenu(ctr.getSize(), fileMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&fileMenu);
   
  vector<string> editMenuContents = {"Edit", "Enable Sheet Music", "Preferences"};
  menu editMenu(ctr.getSize(), editMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&editMenu);
  
  vector<string> viewMenuContents = {"View", "Display Mode:", "Display Song Time:", "Hide Now Line", 
                                     "Hide Measure Line", "Hide Measure Number", "Hide Background", "Show FPS"};
  menu viewMenu(ctr.getSize(), viewMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&viewMenu);
  
  vector<string> displayMenuContents = {"Default", "Line", "Pulse", "Ball", "FFT", "Shade"};
  menu displayMenu(ctr.getSize(), displayMenuContents, TYPE_SUB, 
                   viewMenu.getX() + viewMenu.getWidth(), viewMenu.getItemY(1), &viewMenu, 1);
  ctr.menu->registerMenu(&displayMenu);

  vector<string> songMenuContents = {"Relative", "Absolute"};
  menu songMenu(ctr.getSize(), songMenuContents, TYPE_SUB, 
                viewMenu.getX() + viewMenu.getWidth(), viewMenu.getItemY(2), &viewMenu, 2);
  ctr.menu->registerMenu(&songMenu);

  vector<string> midiMenuContents = {"Midi", "Input", "Output", "Enable Live Play"};
  menu midiMenu(ctr.getSize(), midiMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&midiMenu);

  vector<string> inputMenuContents = {""};
  menu inputMenu(ctr.getSize(), inputMenuContents, TYPE_SUB, 
                 midiMenu.getX() + midiMenu.getWidth(), midiMenu.getItemY(1), &midiMenu, 1);
  ctr.menu->registerMenu(&inputMenu);

  vector<string> outputMenuContents = {""};
  menu outputMenu(ctr.getSize(), outputMenuContents, TYPE_SUB, 
                 midiMenu.getX() + midiMenu.getWidth(), midiMenu.getItemY(2), &midiMenu, 2);
  ctr.menu->registerMenu(&outputMenu);
  
  vector<string> colorMenuContents = {"Color", "Color By:", "Color Scheme:", "Swap Colors", "Invert Color Scheme"};
  menu colorMenu(ctr.getSize(), colorMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&colorMenu);
   
  vector<string> schemeMenuContents = {"Part", "Velocity", "Tonic"};
  menu schemeMenu(ctr.getSize(), schemeMenuContents, TYPE_SUB, 
                  colorMenu.getX() + colorMenu.getWidth(), colorMenu.getItemY(1), &colorMenu, 1);
  ctr.menu->registerMenu(&schemeMenu);
  
  vector<string> infoMenuContents= {"Info", "Program Info", "Help"};
  menu infoMenu(ctr.getSize(), infoMenuContents, TYPE_MAIN, ctr.menu->getOffset(), 0);
  ctr.menu->registerMenu(&infoMenu);

  vector<string> paletteMenuContents = {"Default", "From Background"};
  menu paletteMenu(ctr.getSize(), paletteMenuContents, TYPE_SUB, 
                   colorMenu.getX() + colorMenu.getWidth(), colorMenu.getItemY(2), &colorMenu, 2);
  ctr.menu->registerMenu(&paletteMenu);
  
  vector<string> rightMenuContents = {"Info", "Change Part Color", "Set Tonic"};
  menu rightMenu(ctr.getSize(), rightMenuContents, TYPE_RIGHT, -100,-100); 
  ctr.menu->registerMenu(&rightMenu);
  
  vector<string> colorSelectContents = {"Color Select"};
  menu colorSelect(ctr.getSize(), colorSelectContents, TYPE_COLOR, -100,-100, &rightMenu, 1); 
  ctr.menu->registerMenu(&colorSelect);

  if (argc >= 2) {
    newFile = true;
    filename = argv[1];

    // load an image if supplied
    if (argc == 3) {
      newImage = true;
      imagename = argv[2];
    }
  }

  // main program logic
  while (ctr.getProgramState()) {

    // the now line IS variable
    //nowLineX = ctr.getWidth()/2.0f + ctr.getWidth()/4.0f*sin(GetTime());

    if (newFile) {
      newFile = false;
      run = false;
      timeOffset = 0;
      pauseOffset = 0;

      ctr.load(filename, curFileType,
               nowLine,  showFPS,  showImage,  sheetMusicDisplay,
               measureLine,  measureNumber, 
               colorMode,  displayMode,
               songTimeType,  tonicOffset, 
               zoomLevel);
    }

    if (newImage) {
      newImage = false;
      ctr.image.load(imagename);
    }
    
    if (ctr.getLiveState()) {
      timeOffset = ctr.livePlayOffset;
      ctr.liveInput.update();
      run = false;

      noteData = ctr.liveInput.noteStream;
    }
    else {
      noteData = ctr.file; 
    }

    // fix FPS count bug
    GetFPS();

    // preprocess variables
    clickTmp = -1;
    hoverType.clear();

    // update menu variables
    if (sheetMusicDisplay) {
      songTimePosition.y = 26.0f + ctr.barHeight;
      ctr.topHeight = ctr.barHeight + ctr.menuHeight;
    }
    else {
      songTimePosition.y = 26.0f;
      ctr.topHeight = ctr.menuHeight;
    }
   
    // update hover status 
    hoverDialog(preferenceDisplay,  ctr.getWidth()/2  - ctr.prefWidth/2,
                                    ctr.getHeight()/2 - ctr.prefHeight/2,
                                    ctr.prefWidth,
                                    ctr.prefHeight);
    hoverDialog(infoDisplay,        ctr.getWidth()/2  - ctr.infoWidth/2,
                                    ctr.getHeight()/2 - ctr.infoHeight/2,
                                    ctr.infoWidth,
                                    ctr.infoHeight);
    
    //colorLAB cLAB(colorRGB(121,215,91));
  
    //logQ(colorRGB(cLAB));
    //logQ("l a b:", cLAB.l, cLAB.a, cLAB.b);
    

    //logQ(ctr.getMouseX(), ctr.getMouseY());
    //logQ("focused:", IsWindowFocused());

    // main render loop
    BeginDrawing();
      clearBackground(ctr.bgColor);
    
      if (showImage) {
        ctr.image.draw();
        if (!hoverType.contains(HOVER_DIALOG)) {
          if (ctr.getMousePosition().x > ctr.image.getX() && 
              ctr.getMousePosition().x < ctr.image.getX() + ctr.image.getWidth()) {
            if (ctr.getMousePosition().y > ctr.image.getY() && 
                ctr.getMousePosition().y < ctr.image.getY() + ctr.image.getHeight()) {

              hoverType.add(HOVER_IMAGE); 
            }
          }
        }
      }

      int lastMeasureNum = 0;

      double measureSpacing = measureTextEx(to_string(noteData.measureMap.size() - 1).c_str()).x; 

      if (measureLine || measureNumber) {
        for (unsigned int i = 0; i < noteData.measureMap.size(); i++) {
          float measureLineWidth = 0.5;
          int measureLineY = ctr.menuHeight + (sheetMusicDisplay ? ctr.menuHeight + ctr.sheetHeight : 0);
          double measureLineX = convertSSX(noteData.measureMap[i].getLocation());
          bool hideLine = false;
          if (i && i + 1 < noteData.measureMap.size()) {
            if (convertSSX(noteData.measureMap[i + 1].getLocation()) - measureLineX < 10) {
              hideLine = true;
            }
          }
                
          if (measureLine && !hideLine) {
            if (pointInBox(ctr.getMousePosition(), {int(measureLineX - 3), measureLineY, 6, ctr.getHeight() - measureLineY}) &&
                !hoverType.containsLastFrame(HOVER_MENU) && !hoverType.contains(HOVER_DIALOG)) {
              measureLineWidth = 1;
              hoverType.add(HOVER_MEASURE);
            }
           
            if (!nowLine || fabs(nowLineX - measureLineX) > 3) {  
              drawLineEx(static_cast<int>(measureLineX), measureLineY,
                         static_cast<int>(measureLineX), ctr.getHeight(), measureLineWidth, ctr.bgMeasure);
            }
          }


          if (measureNumber) {
            double lastMeasureLocation = convertSSX(noteData.measureMap[lastMeasureNum].getLocation()); 
            if (!i || lastMeasureLocation + measureSpacing + 10 < measureLineX) {
              // measure number / song time collision detection
              Vector2 songTimeSize = {0,0}; 
              switch (songTimeType) {
                case SONGTIME_ABSOLUTE:
                 songTimeSize = measureTextEx(getSongTime(timeOffset, ctr.getLastTime()).c_str());
                 break;
                case SONGTIME_RELATIVE: 
                 songTimeSize = measureTextEx(getSongPercent(timeOffset, ctr.getLastTime()).c_str());
                 break;
              }
             
              double fadeWidth = 2.0*measureSpacing;
              int measureLineTextAlpha = 255*(min(fadeWidth, ctr.getWidth()-measureLineX))/fadeWidth;

              if (songTimeType != SONGTIME_NONE && measureLineX + 4 < songTimePosition.x*2 + songTimeSize.x) {
                measureLineTextAlpha = max(0.0,min(255.0, 
                                                   255.0 * (1-(songTimePosition.x*2 + songTimeSize.x - measureLineX - 4)/10)));
              }
              else if (measureLineX < fadeWidth) {
                measureLineTextAlpha = 255*max(0.0, (min(fadeWidth, measureLineX+measureSpacing))/fadeWidth);

              }


              int measureTextY = ctr.menuHeight + 4 + (sheetMusicDisplay ? ctr.sheetHeight + ctr.menuHeight : 0);
              drawTextEx(to_string(i + 1).c_str(), measureLineX + 4, measureTextY, ctr.bgLight, measureLineTextAlpha);
              lastMeasureNum = i;
            }
          }
        }
      }
    

      if (nowLine) {
        float nowLineWidth = 0.5;
        int nowLineY = ctr.menuHeight + (sheetMusicDisplay ? ctr.menuHeight + ctr.sheetHeight : 0);
        if (pointInBox(ctr.getMousePosition(), {int(nowLineX - 3), nowLineY, 6, ctr.getHeight() - ctr.barHeight}) && 
            !ctr.menu->mouseOnMenu() && !hoverType.contains(HOVER_DIALOG)) {
          nowLineWidth = 1;
          hoverType.add(HOVER_NOW);
        }
        drawLineEx(nowLineX, nowLineY, nowLineX, ctr.getHeight(), nowLineWidth, ctr.bgNow);
      }

      auto getColorSet = [&](int idx, vector<int>* lp = nullptr) {
        switch(displayMode) {
          case DISPLAY_FFT:
            [[fallthrough]];
          case DISPLAY_BAR:
            [[fallthrough]];
          case DISPLAY_BALL:
            switch (colorMode) {
              case COLOR_PART:
                return (*ctr.getNotes())[idx].track;
              case COLOR_VELOCITY:
                return (*ctr.getNotes())[idx].velocity;
              case COLOR_TONIC:
                return ((*ctr.getNotes())[idx].y - MIN_NOTE_IDX + tonicOffset) % 12 ;
            }
          case DISPLAY_PULSE:
            [[fallthrough]];
          case DISPLAY_LINE:
            switch (colorMode) {
              case COLOR_PART:
                return (*ctr.getNotes())[(*lp)[idx]].track;
              case COLOR_VELOCITY:
                return (*ctr.getNotes())[(*lp)[idx]].velocity;
              case COLOR_TONIC:
                return ((*ctr.getNotes())[(*lp)[idx]].y - MIN_NOTE_IDX + tonicOffset) % 12 ;
            }
        }
        return 0;
      };

      pair<double, double> currentBoundaries = inverseSSX();
      vector<int> curNote;

      // shader <-> display mode map
      switch(displayMode) {
        case DISPLAY_SHADE:
          ctr.beginShaderMode("SH_SHADE"); 
          break;
        default:
          break;
      }

      // note rendering
      for (int i = 0; i < ctr.getNoteCount(); i++) {
        
        if ((*ctr.getNotes())[i].x < currentBoundaries.first*0.9 && (*ctr.getNotes())[i].x > currentBoundaries.second*1.1) {
          continue;
        }
        
        bool noteOn = false;
        
        const auto updateClickIndex = [&](int clickIndex = -1){
          if (!hoverType.contains(HOVER_DIALOG)) {
            noteOn ? clickOnTmp = true : clickOnTmp = false;
            noteOn = !noteOn;
            clickTmp = clickIndex == -1 ? i : clickIndex;
            hoverType.add(HOVER_NOTE);
          }
        };
        
        float cX = convertSSX((*ctr.getNotes())[i].x);
        float cY = convertSSY((*ctr.getNotes())[i].y);
        float cW = (*ctr.getNotes())[i].duration * zoomLevel < 1 ? 1 : (*ctr.getNotes())[i].duration * zoomLevel;
        float cH = (ctr.getHeight() - ctr.menuHeight) / 88;

        switch (displayMode) {
          case DISPLAY_BAR:
            {
              int colorID = getColorSet(i);
              if (cX + cW > 0 && cX < ctr.getWidth()) {
                if ((*ctr.getNotes())[i].isOn ||
                   (timeOffset >= (*ctr.getNotes())[i].x && 
                    timeOffset < (*ctr.getNotes())[i].x + (*ctr.getNotes())[i].duration)) {
                  noteOn = true;
                }
                if (pointInBox(ctr.getMousePosition(), (rect){int(cX), int(cY), int(cW), int(cH)}) && !ctr.menu->mouseOnMenu()) {
                  updateClickIndex();
                }

                auto cSet = noteOn ? colorSetOn : colorSetOff;
                drawRectangle(cX, cY, cW, cH, (*cSet)[colorID]);
              }
            }
            break;
          case DISPLAY_SHADE:
            {
              int colorID = getColorSet(i);
              if (cX + cW > 0 && cX < ctr.getWidth()) {
                if ((*ctr.getNotes())[i].isOn ||
                   (timeOffset >= (*ctr.getNotes())[i].x && 
                    timeOffset < (*ctr.getNotes())[i].x + (*ctr.getNotes())[i].duration)) {
                  noteOn = true;
                }
                if (pointInBox(ctr.getMousePosition(), (rect){int(cX), int(cY), int(cW), int(cH)}) && !ctr.menu->mouseOnMenu()) {
                  updateClickIndex();
                }

                auto cSet = noteOn ? colorSetOn : colorSetOff;
                ctr.setShaderValue("SH_SHADE", "blend_color", (*cSet)[colorID]);
                drawRectangle(cX, cY, cW, cH, {0,0,0});
              }
            }
            break;
          case DISPLAY_BALL:
            {
              int colorID = getColorSet(i);
              float radius = -1 + 2 * (32 - __countl_zero(int(cW)));
              float maxRad = radius;
              float ballY = cY + 2;
              if (cX + cW + radius > 0 && cX - radius < ctr.getWidth()) {
                if (cX < nowLineX - cW) {
                  radius *= 0.3;
                }
                if ((*ctr.getNotes())[i].isOn ||
                   (timeOffset >= (*ctr.getNotes())[i].x && 
                    timeOffset < (*ctr.getNotes())[i].x + (*ctr.getNotes())[i].duration)) {
                  noteOn = true;
                  radius *= (0.3f + 0.7f * (1.0f - float(timeOffset - (*ctr.getNotes())[i].x) / (*ctr.getNotes())[i].duration));
                }
                if (!ctr.menu->mouseOnMenu()) {
                  int realX = 0;
                  if (cX > nowLineX) {
                    realX = cX;
                  }
                  else if (cX + cW < nowLineX) {
                    realX = cX + cW;
                  }
                  else {
                    realX = nowLineX;
                  }
                  if (getDistance(ctr.getMouseX(), ctr.getMouseY(), realX, ballY) < radius) {
                    updateClickIndex();
                  }
                  else if (realX == nowLineX && (getDistance(ctr.getMouseX(), ctr.getMouseY(), cX, ballY) < radius ||
                           pointInBox(ctr.getMousePosition(), 
                                      (rect) {int(cX), int(ballY) - 2, max(int(nowLineX - cX), 0), 4}))) {
                    updateClickIndex();
                  }
                }
                
                
                if (noteOn) {
                  if (cX >= nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, (*colorSetOn)[colorID]);
                  }
  
                  else if (cX + cW < nowLineX) {
                    drawRing({cX + cW, ballY}, radius - 2, radius, (*colorSetOn)[colorID]);
                  }
                  else if (cX < nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, (*colorSetOn)[colorID], 255*radius/maxRad);
                    drawRing({static_cast<float>(nowLineX), ballY}, radius - 2, radius, (*colorSetOn)[colorID]);
                    if (nowLineX - cX > 2 * radius) {
                      drawGradientLineH({cX + radius, ballY + 1}, {static_cast<float>(nowLineX) - radius + 1, ballY + 1}, 
                                        2, (*colorSetOn)[colorID], 255, 255*radius/maxRad);
                    }
                  }
                }
                else {
                  if (cX < nowLineX && cX + cW > nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, (*colorSetOff)[colorID], 255*radius/maxRad);
                    drawRing({static_cast<float>(nowLineX), ballY}, radius - 2, radius, (*colorSetOff)[colorID]);
                    if (nowLineX - cX > 2 * radius) {
                      drawLineEx(cX + radius, ballY + 1, nowLineX - radius, ballY + 1, 2, (*colorSetOff)[colorID]);
                    }
                  }
                  else if (cX < nowLineX) {
                    drawRing({cX + cW, ballY}, radius - 2, radius, (*colorSetOff)[colorID]);
                  }
                  else {
                    drawRing({cX, ballY}, radius - 2, radius, (*colorSetOff)[colorID]);
                  }
                }
                //drawSymbol(SYM_TREBLE, 75, cX,cY, (*colorSetOff)[colorID]);
              }
            }
            break;
          case DISPLAY_LINE:
            {
              vector<int>* linePositions;
              if (i == 0 && !ctr.getLiveState()) {
                linePositions = noteData.getLineVerts();
              }
              else if (ctr.getLiveState()) {
                vector<int> linePosRaw = (*ctr.getNotes())[i].getLinePositions(&(*ctr.getNotes())[i], 
                                                                           (*ctr.getNotes())[i].getNextChordRoot());

                linePositions = &linePosRaw;
              }
              else { 
                break;
              }
              if (!ctr.getLiveState() || (convertSSX((*ctr.getNotes())[i].getNextChordRoot()->x) > 0 && cX < ctr.getWidth())) {
                if ((*ctr.getNotes())[i].isChordRoot()) {
                  for (unsigned int j = 0; j < linePositions->size(); j += 5) {
                    int colorID = getColorSet(j,linePositions);
                    float convSS[4] = {
                                        static_cast<float>(convertSSX((*linePositions)[j+1])),
                                        static_cast<float>(convertSSY((*linePositions)[j+2])),
                                        static_cast<float>(convertSSX((*linePositions)[j+3])),
                                        static_cast<float>(convertSSY((*linePositions)[j+4]))
                                      };

                    if (convSS[2] < 0 || convSS[0] > ctr.getWidth()) {
                       continue;
                    }

                    noteOn = convSS[0] <= nowLineX && convSS[2] > nowLineX;
                    if (pointInBox(ctr.getMousePosition(), 
                                   pointToRect(
                                                {static_cast<int>(convSS[0]), static_cast<int>(convSS[1])},
                                                {static_cast<int>(convSS[2]), static_cast<int>(convSS[3])}
                                              ))) {
                      updateClickIndex((*linePositions)[j]);
                    }
                    auto cSet = noteOn ? colorSetOn : colorSetOff;
                    if (convSS[2] - convSS[0] > 3) {
                      drawLineBezier(convSS[0], convSS[1], convSS[2], convSS[3],
                                 2, (*cSet)[colorID]);
                    }
                    else {
                      drawLineEx(convSS[0], convSS[1], convSS[2], convSS[3],
                                 2, (*cSet)[colorID]);
                    }
                  }
                }
              }
            }
            break;

          case DISPLAY_PULSE:
            {
              vector<int>* linePositions;
              if (i == 0 && !ctr.getLiveState()) {
                linePositions = noteData.getLineVerts();
              }
              else if (ctr.getLiveState()) {
                vector<int> linePosRaw = (*ctr.getNotes())[i].getLinePositions(&(*ctr.getNotes())[i], 
                                                                           (*ctr.getNotes())[i].getNextChordRoot());
                linePositions = &linePosRaw;
              }
              else { 
                break;
              }
              if (!ctr.getLiveState() || (convertSSX((*ctr.getNotes())[i].getNextChordRoot()->x) > 0 && cX < ctr.getWidth())) {
                if ((*ctr.getNotes())[i].isChordRoot()) {
                  for (unsigned int j = 0; j < linePositions->size(); j += 5) {
                    int colorID = getColorSet(j,linePositions);
                    float convSS[4] = {
                                        static_cast<float>(convertSSX((*linePositions)[j+1])),
                                        static_cast<float>(convertSSY((*linePositions)[j+2])),
                                        static_cast<float>(convertSSX((*linePositions)[j+3])),
                                        static_cast<float>(convertSSY((*linePositions)[j+4]))
                                      };

                    if (convSS[2] < 0 || convSS[0] > ctr.getWidth()) {
                       continue;
                    }

                    noteOn = convSS[0] <= nowLineX && convSS[2] > nowLineX;

                    if (pointInBox(ctr.getMousePosition(), 
                                   pointToRect(
                                                {static_cast<int>(convSS[0]), static_cast<int>(convSS[1])},
                                                {static_cast<int>(convSS[2]), static_cast<int>(convSS[3])}
                                              ))) {
                      updateClickIndex((*linePositions)[j]);
                    }
                    auto cSet = noteOn ? colorSetOn : colorSetOff;
                    double nowRatio = (nowLineX-convSS[0])/(convSS[2]-convSS[0]);
                    if (noteOn || clickTmp == (*linePositions)[j]) {
                      double newY = (convSS[3]-convSS[1])*nowRatio + convSS[1];
                      bool nowNote = clickTmp == (*linePositions)[j] ? false : noteOn;
                      drawLineEx(nowNote ? nowLineX - floatLERP(0, (nowLineX-convSS[0])/2.0, nowRatio, INT_SINE) : convSS[0],
                                 nowNote ? newY     - floatLERP(0, (newY-convSS[1])/2.0, nowRatio, INT_SINE) : convSS[1], 
                                 nowNote ? nowLineX - floatLERP(0, (nowLineX-convSS[2])/2.0, nowRatio, INT_ISINE) : convSS[2], 
                                 nowNote ? newY     - floatLERP(0, (newY-convSS[3])/2.0, nowRatio, INT_ISINE) : convSS[3], 
                                 3, (*cSet)[colorID]);
                      drawRing({float(nowNote ? nowLineX - floatLERP(0, (nowLineX-convSS[0])/2.0, nowRatio, INT_SINE) 
                                              : convSS[0]),
                                float(nowNote ? newY - floatLERP(0, (newY-convSS[1])/2.0, nowRatio, INT_SINE) 
                                              : convSS[1])}, 
                               0, 1.5, (*cSet)[colorID]);
                      drawRing({float(nowNote ? nowLineX - floatLERP(0, (nowLineX-convSS[2])/2.0, nowRatio, INT_ISINE) 
                                              : convSS[0]),
                                float(nowNote ? newY - floatLERP(0, (newY-convSS[3])/2.0, nowRatio, INT_ISINE) 
                                              : convSS[1])}, 
                               0, 1.5, (*cSet)[colorID]);
                    }
                    
                    if (convSS[2] >= nowLineX) {
                      double ringFadeAlpha = noteOn ? 255*(1-nowRatio) : 255;
                      drawRing({convSS[0], convSS[1]},
                               0, 3, (*cSet)[colorID], ringFadeAlpha);
                    }
                    if (convSS[2] <= nowLineX) {
                      drawRing({convSS[2], convSS[3]},
                               0, 3, (*cSet)[colorID]);
                    }

                    int ringLimit = 400;
                    int ringDist = timeOffset - (*linePositions)[j+1];

                    double ringRatio = ringDist/static_cast<double>(ringLimit); 
                    if (run && (*linePositions)[j+1] < pauseOffset && timeOffset >= pauseOffset) {
                      ringRatio = 0;
                    }
                    else if (ctr.getPauseTime()<1 && timeOffset == pauseOffset){// || (*linePositions)[j+1] >= pauseOffset) {
                      // this effect has a run-down time of 1 second
                      ringRatio += min(1-ringRatio, ctr.getPauseTime());
                    }
                    else if ((*linePositions)[j+1] < pauseOffset && timeOffset == pauseOffset) {
                      ringRatio = 0;
                      //ringRatio *= max(ctr.getRunTime(), 1.0);
                    }
                    //logQ(timeOffset, ((*linePositions)[j+1], (*linePositions)[j+2]));
                    if (ringDist <= ringLimit && ringDist > 4) {
                      int noteLen = (*ctr.getNotes())[(*linePositions)[j]].duration * zoomLevel < 1 ? 
                                  1 : (*ctr.getNotes())[(*linePositions)[j]].duration * zoomLevel;
                      noteLen = noteLen ? 32 - __countl_zero(noteLen) : 0;
                      double ringRad = floatLERP(6, 5*noteLen, ringRatio, INT_ILINEAR);

                      if (ringRatio > 0) {
                        drawRing({convSS[0], convSS[1]},
                                 ringRad-3, ringRad, 
                                 colorLERP((*colorSetOn)[colorID], (*colorSetOff)[colorID], ringRatio, INT_ICIRCULAR),
                                 floatLERP(0,255, ringRatio, INT_ICIRCULAR));
                      }

                    }
                  }
                }
              }
            }
            break;
          case DISPLAY_FFT:
            if (cX + cW > 0 && cX < ctr.getWidth()) {
              bool drawFFT = false;
              double fftStretchRatio = 1.78;          //TODO: make fft-selection semi-duration-invariant
              int colorID = getColorSet(i); 
              if ((*ctr.getNotes())[i].isOn ||
                 (timeOffset >= (*ctr.getNotes())[i].x && 
                  timeOffset < (*ctr.getNotes())[i].x + (*ctr.getNotes())[i].duration)) {
                noteOn = true;
                drawFFT = true;
              }
              else if ((timeOffset >= (*ctr.getNotes())[i].x + (*ctr.getNotes())[i].duration && 
                        timeOffset < (*ctr.getNotes())[i].x + fftStretchRatio*(*ctr.getNotes())[i].duration) ||
                       (timeOffset < (*ctr.getNotes())[i].x && 
                        timeOffset >= (*ctr.getNotes())[i].x - ctr.getMinTickLen())) {
                
                drawFFT = true;
              }
              if (pointInBox(ctr.getMousePosition(), (rect){int(cX), int(cY), int(cW), int(cH)}) && !ctr.menu->mouseOnMenu()) {
                updateClickIndex();
              }
              if(drawFFT) {
                curNote.push_back(i);
              }

              auto cSet = noteOn ? colorSetOn : colorSetOff;
              drawRectangle(cX, cY, cW, cH, (*cSet)[colorID]);
            }
              break;
        }
      }

      // render FFT lines after notes
      if (displayMode == DISPLAY_FFT) {
        for (const auto& idx : curNote) {
          double freq = getFundamental((*ctr.getNotes())[idx].y);
          int colorID = getColorSet(idx);
          double nowRatio = (timeOffset-(*ctr.getNotes())[idx].x)/((*ctr.getNotes())[idx].duration);
          double pitchRatio = 0;
          if (nowRatio > -0.25 && nowRatio < 0) {
            pitchRatio = 16*pow(nowRatio+0.25,2);
          }
          else if (nowRatio < 1) {
            pitchRatio = 1-1.8*pow(nowRatio,2)/4.5;
          }
          else if (nowRatio < 1.78) {                   // TODO: smoothen interpolation functions
            pitchRatio = 1.0*pow(nowRatio-1.78,2);      // TODO: make function duration-invariant
          }
          int binScale = 2+5*(1+log(1+(*ctr.getNotes())[idx].duration)) +
                         (15.0/128)*(((*ctr.getNotes())[idx].velocity)+1);

          //logQ((*ctr.getNotes())[idx].y, freq, ctr.fftbins.size());
          // simulate harmonics
          constexpr double harmonics[5] = {0.25, 0.5, 1, 2, 4};
          constexpr double harmonicsCoefficient[5] = {0.04, 0.1, 1, 0.1, 0.04};
          constexpr int harmonicsSize = 5;


          for (unsigned int bin = 0; bin < ctr.fftbins.size(); ++bin) {
            ctr.fftbins[bin].second = 0;
          }


          bool foundNote = false;
          for (unsigned int harmonicScale = 0; harmonicScale < harmonicsSize; ++harmonicScale) {
            for (unsigned int bin = 0; bin < ctr.fftbins.size(); ++bin) {
              if (freq*harmonics[harmonicScale] < FFT_MIN_FREQ ||
                  freq*harmonics[harmonicScale] > FFT_MAX_FREQ) {
                continue;
              }
              double fftBinLen = harmonicsCoefficient[harmonicScale]*binScale*pitchRatio * 
                                 fftAC(freq*harmonics[harmonicScale], ctr.fftbins[bin].first);
              
              // pseudo-random numerically stable offset
              
              // TODO: implement spectral rolloff in decay (based on BIN FREQ v. spectral rolloff curve)
              
              fftBinLen *= 1+0.7*pow(((ctr.getPSR() ^ static_cast<int>(ctr.fftbins[bin].first)) % 
                                      static_cast<int>(ctr.fftbins[bin].first)) / ctr.fftbins[bin].first - 0.5, 2);
              
              int startX = FFT_BIN_WIDTH*(bin + 1);
             
              auto cSet = colorSetOn;

              //collision
              if (!foundNote && !hoverType.contains(HOVER_DIALOG) && pointInBox(ctr.getMousePosition(), 
                             {startX-3, static_cast<int>(ctr.getHeight()-ctr.fftbins[bin].second-fftBinLen), 
                              7,        static_cast<int>(fftBinLen)})) {
                foundNote = true;
                cSet = colorSetOff;
                hoverType.add(HOVER_NOTE);
                clickOnTmp = true;
                clickTmp = idx;

              }
              
              drawLineEx(startX,ctr.getHeight()-ctr.fftbins[bin].second,
                         startX,ctr.getHeight()-ctr.fftbins[bin].second-fftBinLen,1, (*cSet)[colorID]);
              ctr.fftbins[bin].second += fftBinLen;
            }
          }
        }
      }
      
      // shader <-> display mode map
      switch(displayMode) {
        case DISPLAY_SHADE:
          ctr.endShaderMode(); 
          break;
        default:
          break;
      }

      // nothing past this point is subject to display shader rendering

      // menu bar rendering
      drawRectangle(0, 0, ctr.getWidth(), ctr.menuHeight, ctr.bgMenu);  

      // sheet music layout
      if (sheetMusicDisplay) {

        // bg
        drawRectangle(0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight, ctr.bgSheet);  
        if (pointInBox(ctr.getMousePosition(), {0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight}) &&
            !hoverType.contains(HOVER_DIALOG)) {
          hoverType.add(HOVER_SHEET);
        }

        // stave lines
        for (int i = 0; i < 2; i++) {
          for (int j = 0; j < 5; j++) {
            drawLineEx(ctr.sheetSideMargin, 
                       ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth,
                       ctr.getWidth() - ctr.sheetSideMargin, 
                       ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth, 
                       1, ctr.bgSheetNote);
          }
        }
        
        //// end lines
        drawLineEx(ctr.sheetSideMargin, ctr.menuHeight + ctr.barMargin, ctr.sheetSideMargin,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);
        drawLineEx(ctr.getWidth() - ctr.sheetSideMargin, ctr.menuHeight + ctr.barMargin, ctr.getWidth() - ctr.sheetSideMargin,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgSheetNote);

        
        drawSymbol(SYM_STAFF_BRACE, 480, 17.0f, float(ctr.menuHeight + ctr.barMargin) - 120, ctr.bgSheetNote);
        drawSymbol(SYM_CLEF_TREBLE, 155, 40.0f, ctr.menuHeight + ctr.barMargin - 47, ctr.bgSheetNote);
        drawSymbol(SYM_CLEF_BASS, 155, 40.0f, float(ctr.menuHeight + ctr.barSpacing + ctr.barMargin - 67), ctr.bgSheetNote);
      

        if (noteData.measureMap.size() != 0) {
          noteData.sheetData.drawSheetPage();
        }

       
        if (IsKeyPressed(KEY_F)) {
          

        }
        //noteData.sheetData.findSheetPages();
        //logQ("cloc", ctr.getCurrentMeasure(timeOffset));
        //logQ("cloc", formatPair(noteData.sheetData.findSheetPageLimit(ctr.getCurrentMeasure(timeOffset))));
      }
      

      // option actions
      switch (songTimeType) {
        case SONGTIME_RELATIVE:
          drawTextEx(getSongPercent(timeOffset, ctr.getLastTime()).c_str(), songTimePosition, ctr.bgLight);
          break;
        case SONGTIME_ABSOLUTE:
          drawTextEx(getSongTime(timeOffset, ctr.getLastTime()).c_str(), songTimePosition, ctr.bgLight);
          break;
      }

      if (showFPS) {
        if (GetTime() - (int)GetTime() < GetFrameTime()) {
          FPSText = to_string(GetFPS());
        }
        drawTextEx(FPSText.c_str(),
                   ctr.getWidth() - 
                  measureTextEx(FPSText.c_str()).x - 4, 4, ctr.bgDark);
      }

      ctr.menu->renderAll();


      if (preferenceDisplay) {
        const int prefSideMargin = ctr.getWidth() - ctr.prefWidth;
        const int prefTopMargin = ctr.getHeight() - ctr.prefHeight;
        drawRectangle(prefSideMargin/2, prefTopMargin/2, ctr.prefWidth, ctr.prefHeight, ctr.bgMenu);  
        

        drawTextEx("Preferences", prefSideMargin/2 + 6, prefTopMargin/2 + 6, ctr.bgDark, 255, 18);
      }
      
      if (infoDisplay) {
        const int infoSideMargin = ctr.getWidth() - ctr.infoWidth;
        const int infoTopMargin = ctr.getHeight() - ctr.infoHeight;
        drawRectangle(infoSideMargin/2, infoTopMargin/2, ctr.infoWidth, ctr.infoHeight, ctr.bgMenu);  
       

        int iconTextSize = 30;
        int iconTextAdjust = 10;
        double borderMargin = 20;
        double iconScale = 0.3;
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
        
        string copySym = "©";
        string copy = " iika-re 2020-" + string(COPY_YEAR);
        string license = "Licensed under GPLv3";

        int copySymSize = 22;
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


      //logQ("bounds", formatPair(inverseSSX()));
      //drawSymbol(SYM_REST_16, 155, convertSSX(inverseSSX().first), 320, ctr.bgNow);
      //drawSymbol(SYM_CLEF_TREBLE, 155, 
                 //convertSSX(inverseSSX().second)-noteData.sheetData.getGlyphWidth(SYM_CLEF_TREBLE,155), 320, ctr.bgNow);
    
    EndDrawing();


    // NOTE: only for debugging sheet area
    if(IsKeyPressed(KEY_TAB)) {
      sheetMusicDisplay = !sheetMusicDisplay;
      sheetMusicDisplay ? ctr.barHeight = ctr.menuHeight + ctr.sheetHeight : ctr.barHeight = ctr.menuHeight;
    }
    // key actions
    if (run) {
      if (timeOffset + GetFrameTime() * 500 < ctr.getLastTime()) {
        timeOffset += GetFrameTime() * 500;
      }
      else {
        timeOffset = ctr.getLastTime();
        run = false;
        pauseOffset = timeOffset;
      }
    }

    if (colorMove) {
      if (colorSquare) {
        colorSelect.setSquare();
      }
      if (colorCircle) {
        colorSelect.setAngle();
      }
      switch (selectType) {
        case SELECT_NOTE:
          if (clickOn) {
            ctr.setTrackOn[(*ctr.getNotes())[clickNote].track] = colorSelect.getColor();
          }
          else {
            ctr.setTrackOff[(*ctr.getNotes())[clickNote].track] = colorSelect.getColor();
          }
          break;
        case SELECT_BG:
          ctr.bgColor = colorSelect.getColor();
          break;
        case SELECT_LINE:
          ctr.bgNow = colorSelect.getColor();
          break;
        case SELECT_MEASURE:
          ctr.bgMeasure = colorSelect.getColor();
          break;
        case SELECT_SHEET:
          ctr.bgSheet = colorSelect.getColor();
          break;
        case SELECT_NONE:
          break;
        default:
          selectType = SELECT_NONE;
          break;
      }
    }
    
    // key logic
    
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SHIFT)) && GetMouseWheelMove() != 0) {
      if (ctr.image.exists() && showImage) {

        // defaults to ±1, adjusted depending on default image scale value
        float scaleModifier = 0.02f;
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
          scaleModifier = 0.002f;
        }
        ctr.image.changeScale(scaleModifier*GetMouseWheelMove());
      }
    }
    
    if ((!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_SHIFT)) && 
        (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) || GetMouseWheelMove() != 0)) {
      if (IsKeyPressed(KEY_DOWN) || (GetMouseWheelMove() < 0)) {
        if (zoomLevel > 0.00001) {
          zoomLevel *= 0.75;
        }
      }
      else {
        if (zoomLevel < 1.2) {
          zoomLevel *= 1.0/0.75;
        }
      }
    }
    if (IsKeyPressed(KEY_SPACE)) {
      if (timeOffset != ctr.getLastTime()) {
        run = !run; 
        pauseOffset = timeOffset;
      }
    }
    if (IsKeyDown(KEY_LEFT)) {
      if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (timeOffset > shiftC * 60) {
          timeOffset -= shiftC * 60;
        }
        else if (timeOffset > 0) {
          timeOffset = 0;
        }
      }
      else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        // immediately move to previous measure
        //
        //logQ(noteData.findMeasure(nowLineX));
        bool measureFirst = true;
        for (unsigned int i = noteData.measureMap.size()-1; i > 0; --i) {
          double measureLineX = convertSSX(noteData.measureMap[i].getLocation());
          if (measureLineX < nowLineX-1) {
            timeOffset = unconvertSSX(measureLineX);
            measureFirst = false;
            break;
          }
        }
        if (measureFirst) {
          timeOffset = 0;
        }
      }
      else {
        if (timeOffset > shiftC * 6) {
          timeOffset -= shiftC * 6;
        }
        else if (timeOffset > 0) {
          timeOffset = 0;
        }
      }
    }
    if (IsKeyDown(KEY_RIGHT)) {
      if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (timeOffset + shiftC * 60 < ctr.getLastTime()) {
          timeOffset += shiftC * 60;
        }
        else if (timeOffset < ctr.getLastTime()) {
          timeOffset = ctr.getLastTime();
        }
      }
      else if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        // immediately move to next measure
        bool measureLast = true;
        for (auto& measure : noteData.measureMap) {
          double measureLineX = convertSSX(measure.getLocation());
          if (measureLineX > nowLineX+1) {
            timeOffset = unconvertSSX(measureLineX);
            measureLast = false;
            break;
          }
        }
        if (measureLast) {
          timeOffset = ctr.getLastTime();
        }
      }
      else {
        if (timeOffset + shiftC * 6 < ctr.getLastTime()) {
          timeOffset += shiftC * 6;
        }
        else {
          timeOffset = ctr.getLastTime();
        }
      }
    }
    if (IsKeyDown(KEY_HOME)) {
      timeOffset = 0;
    }
    if (IsKeyDown(KEY_END)) {
      timeOffset = ctr.getLastTime();
      pauseOffset = timeOffset;
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {

      if (!hoverType.contains(HOVER_DIALOG)) {
        preferenceDisplay = infoDisplay = false;
      }

      switch(fileMenu.getActiveElement()) {
        case -1:
          fileMenu.render = false;
          break;
        case 0:
          fileMenu.render = !fileMenu.render;
          break;
        default:
          if (!fileMenu.render) {
            break;
          }
          switch(fileMenu.getActiveElement()) {
            case 1:
              filenameC = ctr.fileDialog(OSDIALOG_OPEN, filetypes);
              
              if (filenameC != nullptr) {
                filename = static_cast<string>(filenameC);
                newFile = true; 
              }
              free(filenameC);

              ctr.menu->hideAll();
              break;
            case 2:
              imagenameC = ctr.fileDialog(OSDIALOG_OPEN, imagetypes);
              if (imagenameC != nullptr) {
                imagename = static_cast<string>(imagenameC);
                newImage = true;
                
                showImage = true; 
                viewMenu.setContent("Hide Background", 4);
              }
              free(imagenameC);

              break;
            case 3:
              if (ctr.getPlayState()) {
                break; 
              }
              if (curFileType == FILE_MKI) {
                ctr.save(savename, nowLine, showFPS, showImage, sheetMusicDisplay, measureLine, measureNumber,
                         colorMode, displayMode, songTimeType, tonicOffset, zoomLevel);
                break;
              }
              [[fallthrough]];
            case 4:
              if (!ctr.getPlayState() && curFileType != FILE_NONE) {
                savenameC = ctr.fileDialog(OSDIALOG_SAVE, savetypes);
                if (savenameC != nullptr) {
                  savename = static_cast<string>(savenameC);
               
                  if (savename.size() < 4 || savename.substr(savename.size() - 4) != ".mki") {
                    savename += ".mki";
                  }


                  ctr.save(savename, nowLine, showFPS, showImage, sheetMusicDisplay, measureLine, measureNumber,
                           colorMode, displayMode, songTimeType, tonicOffset, zoomLevel);
                  curFileType = FILE_MKI;
                }
                free(savenameC);
              }
              break;
            case 5:
                ctr.setCloseFlag(); 
              break;
          }
          break;
      }
      switch(editMenu.getActiveElement()) {
        case -1:
          editMenu.render = false;
          break;
        case 0:
          editMenu.render = !editMenu.render;
          break;
        default:
          if (!editMenu.render) {
            break;
          }
          switch(editMenu.getActiveElement()) {
            case 1:
              if (editMenu.getContent(1) == "Disable Sheet Music") {
                editMenu.setContent("Enable Sheet Music", 1);
              }
              else if (editMenu.getContent(1) == "Enable Sheet Music") {
                editMenu.setContent("Disable Sheet Music", 1);
              }
              sheetMusicDisplay = !sheetMusicDisplay;
              sheetMusicDisplay ? ctr.barHeight = ctr.menuHeight + ctr.sheetHeight : ctr.barHeight = ctr.menuHeight;
              break;
            case 2:
              preferenceDisplay = !preferenceDisplay;
              break;
            case 3:
              break;
            case 4:
              break;
            case 5:
              break;
          }
          break;
      }
      switch(displayMenu.getActiveElement()) {
        case -1:
          if (!displayMenu.parentOpen() || displayMenu.parent->getActiveElement() == -1) {
            displayMenu.render = false;
          }
          break;
        default:
          if (!displayMenu.render) {
            break;
          }
          switch(displayMenu.getActiveElement()) {
            case 0:
              displayMode = DISPLAY_BAR;
              break;
            case 1:
              displayMode = DISPLAY_LINE;
              break;
            case 2:
              displayMode = DISPLAY_PULSE;
              break;
            case 3:
              displayMode = DISPLAY_BALL;
              break;
            case 4:
              displayMode = DISPLAY_FFT;
              break;
            case 5:
              displayMode = DISPLAY_SHADE;
              break;
            case 6:
              break;
            case 7:
              break;
            case 8:
              break;
            case 9:
              break;
          }
          break;
      }
      switch(songMenu.getActiveElement()) {
        case -1:
          if (!songMenu.parentOpen() || songMenu.parent->getActiveElement() == -1) {
            songMenu.render = false;
          }
          break;
        default:
          if (!songMenu.render) {
            break;
          }
          switch(songMenu.getActiveElement()) {
            case 0:
              songTimeType = SONGTIME_RELATIVE;
              viewMenu.setContent("Hide Song Time", 2);
              break;
            case 1:
              songTimeType = SONGTIME_ABSOLUTE;
              viewMenu.setContent("Hide Song Time", 2);
              break;
            case 2:
              break;
            case 3:
              break;
            case 4:
              break;
            case 5:
              break;
          }
          break;
      }
      switch(viewMenu.getActiveElement()) {
        case -1:
          if (!viewMenu.childOpen()) {
            viewMenu.render = false;
          }
          break;
        case 0:
          viewMenu.render = !viewMenu.render;
          break;
        default:
          if (!viewMenu.render) {
            break;
          }
          switch(viewMenu.getActiveElement()) {
            case 1:
              if (viewMenu.childOpen() && displayMenu.render == false) {
                viewMenu.hideChildMenu();
                displayMenu.render = true;
              }
              else {
                displayMenu.render = !displayMenu.render;
              }
              break;
            case 2:
              if (viewMenu.childOpen() && songMenu.render == false) {
                viewMenu.hideChildMenu();
                songMenu.render = true;
              }
              else {
                if (viewMenu.getContent(2) == "Hide Song Time") {
                  viewMenu.setContent("Display Song Time:", 2);  
                  songTimeType = SONGTIME_NONE;
                }
                else {
                  songMenu.render = !songMenu.render;
                }
              }
              break;
            case 3:
              if (viewMenu.getContent(3) == "Show Now Line") {
                viewMenu.setContent("Hide Now Line", 3);
              }
              else if (viewMenu.getContent(3) == "Hide Now Line") {
                viewMenu.setContent("Show Now Line", 3);
              }
              nowLine = !nowLine;
              break;
            case 4:
              if (viewMenu.getContent(4) == "Show Measure Line") {
                viewMenu.setContent("Hide Measure Line", 4);
              }
              else if (viewMenu.getContent(4) == "Hide Measure Line") {
                viewMenu.setContent("Show Measure Line", 4);
              }
              measureLine = !measureLine;
              break;
            case 5:
              if (viewMenu.getContent(5) == "Show Measure Number") {
                viewMenu.setContent("Hide Measure Number", 5);
              }
              else if (viewMenu.getContent(5) == "Hide Measure Number") {
                viewMenu.setContent("Show Measure Number", 5);
              }
              measureNumber = !measureNumber;
              break;
            case 6:
              if (viewMenu.getContent(6) == "Show Background") {
                viewMenu.setContent("Hide Background", 6);
              }
              else if (viewMenu.getContent(6) == "Hide Background") {
                viewMenu.setContent("Show Background", 6);
              }
              showImage = !showImage;
              break;
            case 7:
              if (viewMenu.getContent(7) == "Show FPS") {
                viewMenu.setContent("Hide FPS", 7);
              }
              else if (viewMenu.getContent(7) == "Hide FPS") {
                viewMenu.setContent("Show FPS", 7);
              }
              showFPS = !showFPS;
              FPSText = to_string(GetFPS());
              break;
          }
          break;
      }
      switch(inputMenu.getActiveElement()) {
        case -1:
          if (!inputMenu.parentOpen() || inputMenu.parent->getActiveElement() == -1) {
            inputMenu.render = false;
          }
          break;
        default:
          if (!inputMenu.render) {
            break;
          }
          ctr.liveInput.openPort(inputMenu.getActiveElement());
          break;
      }
      switch(outputMenu.getActiveElement()) {
        case -1:
          if (!outputMenu.parentOpen() || outputMenu.parent->getActiveElement() == -1) {
            outputMenu.render = false;
          }
          break;
        default:
          if (!outputMenu.render) {
            break;
          }
          ctr.output.openPort(outputMenu.getActiveElement());
          break;
      }
      switch(midiMenu.getActiveElement()) {
        case -1:
          if (!midiMenu.childOpen()) {
            midiMenu.render = false;
          }
          break;
        case 0:
          midiMenu.render = !midiMenu.render;
          break;
        default:
          if (!midiMenu.render) {
            break;
          }
          switch(midiMenu.getActiveElement()) {
            case 1:
              inputMenu.update(ctr.liveInput.getPorts());
              if (midiMenu.childOpen() && !inputMenu.render) {
                midiMenu.hideChildMenu();
                inputMenu.render = true;
              }
              else {
                inputMenu.render = !inputMenu.render;
              }
              break;
            case 2:
              outputMenu.update(ctr.output.getPorts());
              if (midiMenu.childOpen() && !outputMenu.render) {
                midiMenu.hideChildMenu();
                outputMenu.render = true;
              }
              else {
                outputMenu.render = !outputMenu.render;
              }
              break;
            case 3:
              if (midiMenu.getContent(3) == "Enable Live Play") {
                midiMenu.setContent("Disable Live Play", 3);
                zoomLevel *= 3;
              }
              else if (midiMenu.getContent(3) == "Disable Live Play") {
                midiMenu.setContent("Enable Live Play", 3);
                zoomLevel *= 1.0/3.0;
              }
              ctr.toggleLivePlay();
              if (!ctr.getLiveState() && curFileType == FILE_NONE) {
                timeOffset = 0;
              }
              break;
            case 4:
              break;
            case 5:
              break;
          }
          break;
      }
      switch(schemeMenu.getActiveElement()) {
        case -1:
          if (!schemeMenu.parentOpen() || schemeMenu.parent->getActiveElement() == -1) {
            schemeMenu.render = false;
          }
          break;
        default:
          if (!schemeMenu.render) {
            break;
          }
          switch(schemeMenu.getActiveElement()) {
            case 0:
              colorMode = COLOR_PART;
              colorSetOn = &ctr.setTrackOn;
              colorSetOff = &ctr.setTrackOff;
              break;
            case 1:
              colorMode = COLOR_VELOCITY;
              colorSetOn = &ctr.setVelocityOn;
              colorSetOff = &ctr.setVelocityOff;
              break;
            case 2:
              colorMode = COLOR_TONIC;
              colorSetOn = &ctr.setTonicOn;
              colorSetOff = &ctr.setTonicOff;
              break;
            case 3:
              break;
            case 4:
              break;
            case 5:
              break;
          }
          break;
      }
      switch(paletteMenu.getActiveElement()) {
        case -1:
          if (!paletteMenu.parentOpen() || paletteMenu.parent->getActiveElement() == -1) {
            paletteMenu.render = false;
          }
          break;
        default:
          if (!paletteMenu.render) {
            break;
          }
          switch(paletteMenu.getActiveElement()) {
            case 0:
              getColorScheme(KEY_COUNT, ctr.setVelocityOn, ctr.setVelocityOff);
              getColorScheme(TONIC_COUNT, ctr.setTonicOn, ctr.setTonicOff);
              getColorScheme(ctr.getTrackCount(), ctr.setTrackOn, ctr.setTrackOff, ctr.file.trackHeightMap);
              break;
            case 1:
              if (ctr.image.exists()) {
                getColorSchemeImage(SCHEME_KEY, ctr.setVelocityOn, ctr.setVelocityOff);
                getColorSchemeImage(SCHEME_TONIC, ctr.setTonicOn, ctr.setTonicOff);
                getColorSchemeImage(SCHEME_TRACK, ctr.setTrackOn, ctr.setTrackOff, ctr.file.trackHeightMap);
              }
              else{
                logW(LL_WARN, "attempt to get color scheme from nonexistent image");
              }
              break;

          }
          break;
      }
      switch(colorMenu.getActiveElement()) {
        case -1:
          if (!colorMenu.childOpen()) {
            colorMenu.render = false;
          }
          break;
        case 0:
          colorMenu.render = !colorMenu.render;
          break;
        default:
          if (!colorMenu.render) {
            break;
          }
          switch(colorMenu.getActiveElement()) {
            case 1:
              if (colorMenu.childOpen() && schemeMenu.render == false) {
                colorMenu.hideChildMenu();
                schemeMenu.render = true;
              }
              else {
                schemeMenu.render = !schemeMenu.render;
              }
             break;
             case 2:
               if (colorMenu.childOpen() && paletteMenu.render == false) {
                 colorMenu.hideChildMenu();
                 paletteMenu.render = true;
               }
               else {
                 paletteMenu.render = !paletteMenu.render;
               }
               break;
             case 3:
               swap(colorSetOn, colorSetOff);
               break;
             case 4:
               invertColorScheme(ctr.bgColor, ctr.bgNow, colorSetOn, colorSetOff);
               break;
             case 5:
               break;
          }
          break;
      }
      switch(infoMenu.getActiveElement()) {
        case -1:
          if (!infoMenu.childOpen()) {
            infoMenu.render = false;
          }
          break;
        case 0:
          infoMenu.render = !infoMenu.render;
          break;
        default:
          if (!infoMenu.render) {
            break;
          }
          switch(infoMenu.getActiveElement()) {
            case 1:
              infoDisplay = !infoDisplay;
              break;
            case 2:
              infoMenu.render = false;
              OpenURL(SITE_LINK);
              break;
          }
          break;
      }
      switch(colorSelect.getActiveElement()) {
        case -1:
          if (!colorSelect.parentOpen() || colorSelect.parent->getActiveElement() == -1) {
            colorSelect.render = false;
          }
          break;
        default:
          if (!colorSelect.render) {
            break;
          }
          switch(colorSelect.getActiveElement()) {
            case 0:
              colorMove = true;
              if (pointInBox(ctr.getMousePosition(), colorSelect.getSquare()) || colorSelect.clickCircle(1)) {
                colorSquare = true;
              }
              else if (colorSelect.clickCircle(0)) {
                colorCircle = true;
              }
              break;
            case 1:
              break;
            case 2:
              break;
            case 3:
              break;
            case 4:
              break;
            case 5:
              break;
          }
          break;
      }
      switch(rightMenu.getActiveElement()) {
        case -1:
          if (!rightMenu.childOpen()) {
            rightMenu.render = false;
          }
          else {
          }            
          break;
        default:
          if (!rightMenu.render) {
            break;
          }
          switch(rightMenu.getActiveElement()) {
            case 0:
              if (rightMenu.getSize() == 1) {
                if (rightMenu.childOpen() && colorSelect.render == false) {
                  rightMenu.hideChildMenu();
                }
                else {
                  if (rightMenu.getContent(0) == "Remove Image") {
                    //logQ("attempted to remove image: size:");
                    ctr.image.unloadData();
                    rightMenu.render = false;
                  }
                  else {
                    colorSelect.render = !colorSelect.render;
                  }
                }
              } 
              break;
            case 1:
              if (rightMenu.getSize() == 3) {
                if (rightMenu.childOpen() && colorSelect.render == false) {
                  rightMenu.hideChildMenu();
                }
                else {
                  colorSelect.render = !colorSelect.render;
                }
              }
              break;
            case 2:
              tonicOffset = ((*ctr.getNotes())[clickNote].y - MIN_NOTE_IDX + tonicOffset) % 12;
              break;
          }
          break;
      }
  

      if (!hoverType.contains(HOVER_NOW, HOVER_NOTE, HOVER_MEASURE, HOVER_MENU, HOVER_SHEET, HOVER_DIALOG) && 
          !hoverType.containsLastFrame(HOVER_MENU, HOVER_MEASURE)) {
        if (hoverType.contains(HOVER_IMAGE)) {
          ctr.image.updateBasePosition();
          //logQ("UPDATE BASE");
        }
      }
    }
    //logQ("can image move?:", !hoverType.contains(HOVER_NOW, HOVER_NOTE, HOVER_MEASURE, HOVER_MENU, HOVER_SHEET) && 
          //!hoverType.containsLastFrame(HOVER_MENU, HOVER_MEASURE));
    //logQ("label", rightMenuContents[1], "v.", rightMenu.getContent(0));
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        colorMove = false;
        colorSquare = false;
        colorCircle = false;

        ctr.image.finalizePosition();
    }
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {    
      ctr.image.updatePosition();
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
      clickNote = clickTmp;
      clickOn = clickOnTmp;
      ctr.menu->hideAll();

      if (!ctr.menu->mouseOnMenu()) {
        if (!pointInBox(ctr.getMousePosition(), (rect){0, 0, ctr.getWidth(), 20})) {
          
          int rightX = 0, rightY = 0, colorX = 0, colorY = 0;

          if (clickNote != -1) {
            rightX = round(nowLineX + ((*ctr.getNotes())[clickNote].x - timeOffset) * zoomLevel);
            rightY = (ctr.getHeight() - round((ctr.getHeight() - ctr.menuHeight) * 
                      static_cast<double>((*ctr.getNotes())[clickNote].y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
          }
          
          // find coordinate to draw right click menu
          rightMenu.findMenuLocation(rightX, rightY);
          rightMenu.findColorSelectLocation(colorX, colorY, rightX, rightY);
          
          colorSelect.setXY(colorX, colorY);

          if (clickNote != -1) {
            selectType = SELECT_NOTE;
            rightMenuContents[1] = "Change Part Color";
            rightMenu.update(rightMenuContents);
            rightMenu.setContent(getNoteInfo((*ctr.getNotes())[clickNote].track, 
                                             (*ctr.getNotes())[clickNote].y - MIN_NOTE_IDX), 0);
            
            // set note color for color wheel
            if (clickOn) {
              colorSelect.setColor(ctr.setTrackOn[(*ctr.getNotes())[clickNote].track]);
            }
            else{
              colorSelect.setColor(ctr.setTrackOff[(*ctr.getNotes())[clickNote].track]);
            }
          }
          else {
            if (sheetMusicDisplay && 
                pointInBox(ctr.getMousePosition(), (rect){0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight}) &&
                !hoverType.contains(HOVER_DIALOG)) {
              hoverType.add(HOVER_SHEET);
              selectType = SELECT_SHEET;
              colorSelect.setColor(ctr.bgSheet);
            }
            else if (pointInBox(ctr.getMousePosition(), 
                                {static_cast<int>(nowLineX - 3), ctr.barHeight, 6, ctr.getHeight() - ctr.barHeight}) &&
                     !hoverType.contains(HOVER_DIALOG)) {
              hoverType.add(HOVER_LINE);
              selectType = SELECT_LINE;
              rightMenuContents[1] = "Change Line Color";
              colorSelect.setColor(ctr.bgNow);
            }
            else {
              bool measureSelected = false;
              for (unsigned int i = 0; i < noteData.measureMap.size(); i++) {
                double measureLineX = convertSSX(noteData.measureMap[i].getLocation());
                if (pointInBox(ctr.getMousePosition(), 
                               {static_cast<int>(measureLineX - 3), ctr.barHeight, 6, ctr.getHeight() - ctr.barHeight}) && 
                    !ctr.menu->mouseOnMenu() && !hoverType.contains(HOVER_DIALOG)) {
                  measureSelected = true;
                  break; 
                }
              }
              
              
              if (measureSelected) {
                selectType = SELECT_MEASURE;
                rightMenuContents[1] = "Change Line Color";
                colorSelect.setColor(ctr.bgMeasure);
              }
              else if (ctr.image.exists() &&
                      (!hoverType.contains(HOVER_NOW, HOVER_NOTE, HOVER_MEASURE, HOVER_MENU, HOVER_SHEET, HOVER_DIALOG) && 
                       !hoverType.containsLastFrame(HOVER_MENU) && hoverType.contains(HOVER_IMAGE))) { 
                    selectType = SELECT_NONE; // no color change on image
                    rightMenuContents[1] = "Remove Image";
                    //logQ("rightclicked on image");
              }
              else if (!hoverType.contains(HOVER_DIALOG)) {
                selectType = SELECT_BG;
                rightMenuContents[1] = "Change Color";
                colorSelect.setColor(ctr.bgColor);
              }
              else {
                selectType = SELECT_NONE;
                rightMenuContents[1] = "Empty";
              }
            }
            rightMenu.setContent("", 0);
            auto f = rightMenuContents.begin() + 1;
            auto e = rightMenuContents.end() - 1;

            //logQ(formatVector(rightMenuContents));
            vector<string> newRight(f, e);
            rightMenu.update(newRight);
          }
          rightMenu.setXY(rightX, rightY);
          
          if (!hoverType.contains(HOVER_DIALOG)) {
            preferenceDisplay = infoDisplay = false;
            rightMenu.render = true;
          }
        }
      }
    }
    if (ctr.menu->mouseOnMenu() || pointInBox(ctr.getMousePosition(), {0, 0, ctr.getWidth(), ctr.menuHeight})) {
      hoverType.add(HOVER_MENU);
    }

    ctr.update(timeOffset, nowLineX, run,
               newFile, newImage, filename, imagename);
  }

  ctr.unloadData();
  osdialog_filters_free(filetypes); 
  osdialog_filters_free(savetypes); 
  osdialog_filters_free(imagetypes); 
  CloseWindow();

  return 0;
}
