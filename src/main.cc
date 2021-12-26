#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <raylib.h>
#include "enum.h"
#include "box.h"
#include "log.h"
#include "misc.h"
#include "menu.h"
#include "data.h"
#include "wrap.h"
#include "color.h"
#include "define.h"
#include "menuctr.h"
#include "controller.h"
#include "../dpd/osdialog/osdialog.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::min;
using std::max;
using std::to_string;
using std::thread;
using std::ref;

controller ctr;
Font font;

int main (int argc, char* argv[]) {

 
  // basic window setup

  SetTraceLogLevel(LOG_NONE);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(mWidth, mHeight, (string("nodumi ") + string(mVersion)).c_str());
  SetTargetFPS(60);
  SetExitKey(KEY_F7);
  font = LoadFontEx("bin/fonts/yklight.ttf", 14, 0, 250);
  ctr.loadTextures();
  
 
  // program-wide variables 

  // scaling settings
  double zoomLevel = 0.125;
  double timeOffset = 0;
  const double shiftC = 2.5;

  // play settings
  bool run = false;

  // view settings
  bool nowLine = true;
  bool showFPS = false;
  bool colorMove = false;
  bool colorSquare = false;
  bool colorCircle = false;
  bool sheetMusicDisplay = false;
  
  int songTimeType = SONGTIME_NONE;
  int tonicOffset = 0;
  int displayMode = DISPLAY_BAR;
  
  float nowLineX = ctr.getWidth()/2.0f;

  string FPSText = "";
  
  // color variables
  int selectType = SELECT_NONE;
  int colorMode = COLOR_PART;
  vector<colorRGB>* colorSetOn = &ctr.setTrackOn;
  vector<colorRGB>* colorSetOff = &ctr.setTrackOff;

  // file IO controllers
  bool newFile = false;
  char* filenameC = nullptr;
  string filename = "";
  osdialog_filters* filetypes = osdialog_filters_parse("midi:mid;mki:mki");
  osdialog_filters* savetypes = osdialog_filters_parse("mki:mki");
  osdialog_filters* imagetypes = osdialog_filters_parse("png:png");

  // right click variables
  int clickNote = -1;
  int clickTmp = -1;
  bool clickOn = false;
  bool clickOnTmp = false;

  // menu controller
  menuController menuctr = menuController();

  // sheet music data
  //SetTextureFilter(bass, FILTER_ANISOTROPIC_16X);

  // screen space conversion functions
  const auto convertSSX = [&] (int value) {
    return nowLineX + (value - timeOffset) * zoomLevel;
  };

  const auto convertSSY = [&] (int value) {
    return (ctr.getHeight() - (ctr.getHeight() - (ctr.menuHeight + ctr.barHeight)) *
            static_cast<float>(value - MIN_NOTE_IDX + 3) / (NOTE_RANGE + 4));
  };

  // menu objects
  vector<string> fileMenuContents = {"File", "Open File", "Open Image", "Save", "Save As", "Exit"};
  menu fileMenu(ctr.getSize(), fileMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&fileMenu);
   
  vector<string> editMenuContents = {"Edit", "Enable Sheet Music", "Preferences"};
  menu editMenu(ctr.getSize(), editMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&editMenu);
  
  vector<string> viewMenuContents = {"View", "Display Mode:", "Display Song Time:", "Hide Now Line", "Show Background", "Show FPS"};
  menu viewMenu(ctr.getSize(), viewMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&viewMenu);
  
  vector<string> displayMenuContents = {"Standard", "Line", "Line (Circle)", "Ball"};
  menu displayMenu(ctr.getSize(), displayMenuContents, &viewMenu, TYPE_SUB, viewMenu.getX() + viewMenu.getWidth(), viewMenu.getItemY(1));
  menuctr.registerMenu(&displayMenu);

  vector<string> songMenuContents = {"Relative", "Absolute"};
  menu songMenu(ctr.getSize(), songMenuContents, &viewMenu, TYPE_SUB, viewMenu.getX() + viewMenu.getWidth(), viewMenu.getItemY(2));
  menuctr.registerMenu(&songMenu);

  vector<string> midiMenuContents = {"Midi", "Input", "Output", "Enable Live Play"};
  menu midiMenu(ctr.getSize(), midiMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&midiMenu);

  vector<string> inputMenuContents = {""};
  menu inputMenu(ctr.getSize(), inputMenuContents, &midiMenu, TYPE_SUB, midiMenu.getX() + midiMenu.getWidth(), midiMenu.getItemY(1));
  menuctr.registerMenu(&inputMenu);

  vector<string> colorMenuContents = {"Color", "Color By:", "Color Scheme:", "Swap Colors", "Invert Color Scheme"};
  menu colorMenu(ctr.getSize(), colorMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&colorMenu);
   
  vector<string> schemeMenuContents = {"Part", "Velocity", "Tonic"};
  menu schemeMenu(ctr.getSize(), schemeMenuContents, &colorMenu, TYPE_SUB, colorMenu.getX() + colorMenu.getWidth(), colorMenu.getItemY(1));
  menuctr.registerMenu(&schemeMenu);

  vector<string> paletteMenuContents = {"Default", "From Background"};
  menu paletteMenu(ctr.getSize(), paletteMenuContents, &colorMenu, TYPE_SUB, colorMenu.getX() + colorMenu.getWidth(), colorMenu.getItemY(2));
  menuctr.registerMenu(&paletteMenu);
  
  vector<string> rightMenuContents = {"Info", "Change Part Color", "Set Tonic"};
  menu rightMenu(ctr.getSize(), rightMenuContents, nullptr, TYPE_RIGHT, -100,-100); 
  menuctr.registerMenu(&rightMenu);
  
  vector<string> colorSelectContents = {"Color Select"};
  menu colorSelect(ctr.getSize(), colorSelectContents, &rightMenu, TYPE_COLOR, -100,-100); 
  menuctr.registerMenu(&colorSelect);
  
  /* 
   * * * * * 
   * LOGIC *
   * * * * * 
   */

  if (argc == 2) {
    string filename = argv[1];
    transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    string ext = filename.substr(filename.size() - 3);

    ifstream filecheck;
    filecheck.open(filename);
    if (!filecheck || (ext != "mid" && ext != "mki")) {
      logW(LL_WARN, "invalid filename: " + filename);
    }
    filecheck.close();

    ctr.load(filename);
  }
  
  while (ctr.getProgramState()) {

    if (newFile) {
      newFile = false;
      run = false;
      timeOffset = 0;

      ctr.load(filename);
    }
    
    if (ctr.getLiveState()) {
      timeOffset = ctr.livePlayOffset;
      ctr.liveInput.update();
      run = false;
    }

    // fix FPS count bug
    GetFPS();

    // preprocess variables
    clickTmp = -1;

    // main render loop
    
    BeginDrawing();
      clearBackground(ctr.bgColor);
      
      int lastMeasureNum = 0;
      int measureSpacing = MeasureTextEx(font, to_string(ctr.file.measureMap.size() - 1).c_str(), font.baseSize, 0.5).x;

      for (unsigned int i = 0; i < ctr.file.measureMap.size(); i++) {
        float measureLineWidth = 0.5;
        int measureLineY = ctr.menuHeight + (sheetMusicDisplay ? ctr.menuHeight + ctr.sheetHeight : 0);
        if (convertSSX(ctr.file.measureMap[i].getLocation()) + measureSpacing + 4 > 0) {
          double measureLineX = convertSSX(ctr.file.measureMap[i].getLocation());
          if (pointInBox(GetMousePosition(), {int(measureLineX - 3), measureLineY, 6, ctr.getHeight() - ctr.barHeight}) && !menuctr.mouseOnMenu()) {
            measureLineWidth = 1; 
          }
         
          if (nowLineX != convertSSX(ctr.file.measureMap[i].getLocation())) {  
            drawLineEx(convertSSX(ctr.file.measureMap[i].getLocation()), ctr.menuHeight,
                       convertSSX(ctr.file.measureMap[i].getLocation()), ctr.getHeight(), measureLineWidth, ctr.bgMeasure);
          }



          if (!i || convertSSX(ctr.file.measureMap[lastMeasureNum].getLocation()) + measureSpacing + 10 <
                    convertSSX(ctr.file.measureMap[i].getLocation())) {
            int measureTextY = ctr.menuHeight + 4 + (sheetMusicDisplay ? ctr.sheetHeight : 0);
            drawTextEx(font, to_string(i + 1).c_str(), convertSSX(ctr.file.measureMap[i].getLocation()) + 4, measureTextY, ctr.bgLight);
            lastMeasureNum = i;
          }
        }
      }



      if (nowLine) {
        float nowLineWidth = 0.5;
        int nowLineY = ctr.menuHeight + (sheetMusicDisplay ? ctr.menuHeight + ctr.sheetHeight : 0);
        if (pointInBox(GetMousePosition(), {int(nowLineX - 3), nowLineY, 6, ctr.getHeight() - ctr.barHeight}) && !menuctr.mouseOnMenu()) {
          nowLineWidth = 1;
        }
        drawLineEx(nowLineX, nowLineY, nowLineX, ctr.getHeight(), nowLineWidth, ctr.bgNow);
      }

      // note handling
      for (int i = 0; i < ctr.getNoteCount(); i++) {
        
        int colorID = 0;
        bool noteOn = false;
        
        const auto updateClickIndex = [&] {
          noteOn ? clickOnTmp = true : clickOnTmp = false;
          noteOn = !noteOn;
          clickTmp = i;
        };

        
        float cX = convertSSX(ctr.notes->at(i).x);
        float cY = convertSSY(ctr.notes->at(i).y);
        float cW = ctr.notes->at(i).duration * zoomLevel < 1 ? 1 : ctr.notes->at(i).duration * zoomLevel;
        float cH = (ctr.getHeight() - ctr.menuHeight) / 88;
        
        
        switch (colorMode) {
          case COLOR_PART:
            colorID = ctr.notes->at(i).track;
            break;
          case COLOR_VELOCITY:
            colorID = ctr.notes->at(i).velocity;
            break;
          case COLOR_TONIC:
            colorID = (ctr.notes->at(i).y - MIN_NOTE_IDX + tonicOffset) % 12 ;
            break;
        }
        
        switch (displayMode) {
          case DISPLAY_BAR:
            if (cX + cW > 0 && cX < ctr.getWidth()) {
                

              if (ctr.notes->at(i).isOn ||
                 (timeOffset >= ctr.notes->at(i).x && timeOffset < ctr.notes->at(i).x + ctr.notes->at(i).duration)) {
                noteOn = true;
              }
              if (pointInBox(GetMousePosition(), (rect){int(cX), int(cY), int(cW), int(cH)}) && !menuctr.mouseOnMenu()) {
                updateClickIndex();
              }

              if (noteOn) {
                drawRectangle(cX, cY, cW, cH, colorSetOn->at(colorID));
              }
              else {
                drawRectangle(cX, cY, cW, cH, colorSetOff->at(colorID));
              }
            }
            break;
          case DISPLAY_BALL:
            {
              float radius = 1 + 3 * log(cW);
              float ballY = cY + 2;
              if (cX + cW + radius > 0 && cX - radius < ctr.getWidth()) {

                  if (cX < nowLineX - cW) {
                    radius *= 0.3;
                  }
                if (ctr.notes->at(i).isOn ||
                   (timeOffset >= ctr.notes->at(i).x && timeOffset < ctr.notes->at(i).x + ctr.notes->at(i).duration)) {
                  noteOn = true;
                  radius *= (0.3f + 0.7f * (1.0f - float(timeOffset - ctr.notes->at(i).x) / ctr.notes->at(i).duration));
                }
                if (!menuctr.mouseOnMenu()) {
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
                  if (getDistance(GetMouseX(), GetMouseY(), realX, ballY) < radius) {
                    updateClickIndex();
                  }
                  else if (realX == nowLineX && (getDistance(GetMouseX(), GetMouseY(), cX, ballY) < radius ||
                           pointInBox(GetMousePosition(), (rect) {int(cX), int(ballY) - 2, max(int(nowLineX - cX), 0), 4}))) {
                    updateClickIndex();
                  }
                }
                
                
                if (noteOn) {
                  if (cX >= nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, colorSetOn->at(colorID));
                  }
  
                  else if (cX + cW < nowLineX) {
                    drawRing({cX + cW, ballY}, radius - 2, radius, colorSetOn->at(colorID));
                  }
                  else if (cX < nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, colorSetOn->at(colorID));
                    drawRing({nowLineX, ballY}, radius - 2, radius, colorSetOn->at(colorID));
                    if (nowLineX - cX > 2 * radius) {
                      drawLineEx(cX + radius, ballY + 1, nowLineX - radius, ballY + 1, 2, colorSetOn->at(colorID));
                    }
                  }
                }
                else {
                  if (cX < nowLineX && cX + cW > nowLineX) {
                    drawRing({cX, ballY}, radius - 2, radius, colorSetOff->at(colorID));
                    drawRing({nowLineX, ballY}, radius - 2, radius, colorSetOff->at(colorID));
                    if (nowLineX - cX > 2 * radius) {
                      drawLineEx(cX + radius, ballY + 1, nowLineX - radius, ballY + 1, 2, colorSetOff->at(colorID));
                    }
                  }
                  else if (cX < nowLineX) {
                    drawRing({cX + cW, ballY}, radius - 2, radius, colorSetOff->at(colorID));
                  }
                  else {
                    drawRing({cX, ballY}, radius - 2, radius, colorSetOff->at(colorID));
                  }
                }
              }
            }
            break;
          case DISPLAY_LINE:
            {
              vector<int>* linePositions;
              if (i == 0 && !ctr.getLiveState()) {
                linePositions = ctr.file.getLineVerts();
              }
              else if (ctr.getLiveState()) {
                vector<int> linePosRaw = getLinePositions(&ctr.notes->at(i), ctr.notes->at(i).getNextChordRoot());
                linePositions = &linePosRaw;
              }
              else { 
                break;
              }
              if (!ctr.getLiveState() || (convertSSX(ctr.notes->at(i).getNextChordRoot()->x) > 0 && cX < ctr.getWidth())) {
                if (ctr.notes->at(i).isChordRoot()) {
                  //cerr << i << endl;
                  for (unsigned int j = 0; j < linePositions->size(); j += 5) {
                    switch (colorMode) {
                      case COLOR_PART:
                        colorID = ctr.notes->at(linePositions->at(j)).track;
                        break;
                      case COLOR_VELOCITY:
                        colorID = ctr.notes->at(linePositions->at(j)).velocity;
                        break;
                      case COLOR_TONIC:
                        colorID = (ctr.notes->at(linePositions->at(j)).y - MIN_NOTE_IDX + tonicOffset) % 12 ;
                        break;
                    }
                    if (convertSSX(linePositions->at(j + 1)) <= nowLineX && convertSSX(linePositions->at(j + 3)) > nowLineX) {
                      noteOn = true;
                    }
                    else {
                      noteOn = false;
                    }
                    if (pointInBox(GetMousePosition(), pointToRect({(int)convertSSX(linePositions->at(j + 1)),
                                   (int)convertSSY(linePositions->at(j + 2))}, {(int)convertSSX(linePositions->at(j + 3)),
                                   (int)convertSSY(linePositions->at(j + 4))}))) {
                      updateClickIndex();
                    }
                    if (noteOn) {
                      drawLineEx(convertSSX(linePositions->at(j + 1)), convertSSY(linePositions->at(j + 2)),
                                 convertSSX(linePositions->at(j + 3)), convertSSY(linePositions->at(j + 4)), 
                                 2, colorSetOn->at(colorID));
                    }
                    else {
                      drawLineEx(convertSSX(linePositions->at(j + 1)), convertSSY(linePositions->at(j + 2)),
                                 convertSSX(linePositions->at(j + 3)), convertSSY(linePositions->at(j + 4)), 
                                 2, colorSetOff->at(colorID));
                    }
                  }
                }
              }
            }
            break;

          case DISPLAY_BALLLINE:
            break;
        }
      }

      // menu bar rendering
      drawRectangle(0, 0, ctr.getWidth(), ctr.menuHeight, ctr.bgMenu);  

      // sheet music layout
      if (sheetMusicDisplay) {
        
        // needs refactoring 
        

        // bg
        drawRectangle(0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight, ctr.bgSheet);  

        // stave lines
        for (int i = 0; i < 2; i++) {
          for (int j = 0; j < 5; j++) {
            drawLineEx(30, ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth,
                       ctr.getWidth() - 30, ctr.menuHeight + ctr.barMargin + i * ctr.barSpacing + j * ctr.barWidth, 1, ctr.bgDark);
          }
        }
        
        //// end lines
        drawLineEx(30, ctr.menuHeight + ctr.barMargin, 30,
                   ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgDark);
        //drawLineEx(ctr.getWidth() - 30, ctr.menuHeight + ctr.barMargin, ctr.getWidth() - 30,
                   //ctr.menuHeight + ctr.barMargin + 4 * ctr.barWidth + ctr.barSpacing, 2, ctr.bgDark);

        // static sprites
        DrawTextureEx(ctr.brace, {18.0f, float(ctr.menuHeight + ctr.barMargin) + 2}, 0, 1.0f, {0, 0, 0, 255});
        DrawTextureEx(ctr.treble, {40.0f, ctr.menuHeight + 35.0f}, 0, 1.0f, {0, 0, 0, 255});
        DrawTextureEx(ctr.bass, {40.0f, float(ctr.menuHeight + ctr.barSpacing + ctr.barMargin - 1)}, 0, 1.0f, {0, 0, 0, 255});
        
      }
      

      // option actions
      if (songTimeType == SONGTIME_RELATIVE) {
        if (sheetMusicDisplay) {
          drawTextEx(font, getSongPercent(timeOffset, ctr.getLastTime()).c_str(), 6, 26 + ctr.barHeight, ctr.bgLight);
        }
        else {
          drawTextEx(font, getSongPercent(timeOffset, ctr.getLastTime()).c_str(), 6, 26, ctr.bgLight);
        }
      }
      else if (songTimeType == SONGTIME_ABSOLUTE) {
        if (sheetMusicDisplay) {
          drawTextEx(font, getSongTime(timeOffset, ctr.getLastTime()).c_str(), 6, 26 + ctr.barHeight, ctr.bgLight);
        }
        else {
          drawTextEx(font, getSongTime(timeOffset, ctr.getLastTime()).c_str(), 6, 26, ctr.bgLight);
        }
      }

      if (showFPS) {
        if (GetTime() - (int)GetTime() < GetFrameTime()) {
          FPSText = to_string(GetFPS());
        }
        drawTextEx(font, FPSText.c_str(),
                   ctr.getWidth() - MeasureTextEx(font, FPSText.c_str(), font.baseSize, 0.5).x - 3, 3, ctr.bgDark);
      }

      //fileMenu.draw();
      menuctr.renderAll();

    EndDrawing();

    // key actions
    if (run) {
      if (timeOffset + GetFrameTime() * 500< ctr.getLastTime()) {
        timeOffset += GetFrameTime() * 500;
      }
      else {
        timeOffset = ctr.getLastTime();
        run = false;
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
            ctr.setTrackOn[ctr.notes->at(clickNote).track] = colorSelect.getColor();
          }
          else {
            ctr.setTrackOff[ctr.notes->at(clickNote).track] = colorSelect.getColor();
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
      }
    }
    
    // key logic

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) || GetMouseWheelMove() != 0) {
      if (IsKeyPressed(KEY_DOWN) || GetMouseWheelMove() < 0) {
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
      run = !run; 
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
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
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
              filenameC = osdialog_file(OSDIALOG_OPEN, ".", nullptr, filetypes);
              
              if (filenameC != nullptr) {
                filename = static_cast<string>(filenameC);
                newFile = true; 
              }

              menuctr.hideAll();
              break;
            case 2:
              break;
            case 3:
              break;
            case 4:
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
              displayMode = DISPLAY_BALLLINE;
              break;
            case 3:
              displayMode = DISPLAY_BALL;
              break;
            case 4:
              break;
            case 5:
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
              break;
            case 5:
              if (viewMenu.getContent(5) == "Show FPS") {
                viewMenu.setContent("Hide FPS", 5);
              }
              else if (viewMenu.getContent(5) == "Hide FPS") {
                viewMenu.setContent("Show FPS", 5);
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
              if (midiMenu.childOpen() && inputMenu.render == false) {
                midiMenu.hideChildMenu();
                inputMenu.render = true;
              }
              else {
                inputMenu.update(ctr.liveInput.getPorts());
                inputMenu.render = !inputMenu.render;
              }
              break;
            case 2:
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
              getColorScheme(128, ctr.setVelocityOn, ctr.setVelocityOff);
              getColorScheme(12, ctr.setTonicOn, ctr.setTonicOff);
              getColorScheme(ctr.getTrackCount(), ctr.setTrackOn, ctr.setTrackOff);
              break;
            case 1:
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
              if (pointInBox(GetMousePosition(), colorSelect.getSquare()) || colorSelect.clickCircle(1)) {
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
                  colorSelect.render = !colorSelect.render;
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
              tonicOffset = (ctr.notes->at(clickNote).y - MIN_NOTE_IDX + tonicOffset) % 12;
              break;
          }
          break;
      }


    }
    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON)) {
        colorMove = false;
        colorSquare = false;
        colorCircle = false;
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
      clickNote = clickTmp;
      clickOn = clickOnTmp;
      menuctr.hideAll();

      if (!pointInBox(GetMousePosition(), (rect){0, 0, ctr.getWidth(), 20})) {
        
        int rightX = 0, rightY = 0, colorX = 0, colorY = 0;

        if (clickNote != -1) {
          rightX = round(nowLineX + (ctr.notes->at(clickNote).x - timeOffset) * zoomLevel);
          rightY = (ctr.getHeight() - round((ctr.getHeight() - ctr.menuHeight) * 
                    static_cast<double>(ctr.notes->at(clickNote).y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3)));
        }
        
        // find coordinate to draw right click menu
        getMenuLocation(ctr.getWidth(), ctr.getHeight(), GetMouseX(), GetMouseY(),
                            rightX, rightY, rightMenu.getWidth(), rightMenu.getHeight());
        getColorSelectLocation(ctr.getWidth(), ctr.getHeight(), colorX, colorY,
                               rightX, rightY, rightMenu.getWidth(),
                               rightMenu.getHeight());
        
        colorSelect.setXY(colorX, colorY);

        if (clickNote != -1) {
          selectType = SELECT_NOTE;
          rightMenuContents[1] = "Change Part Color";
          rightMenu.update(rightMenuContents);
          rightMenu.setContent(getNoteInfo(ctr.notes->at(clickNote).track, ctr.notes->at(clickNote).y - MIN_NOTE_IDX), 0);
          
          // set note color for color wheel
          if (clickOn) {
            colorSelect.setColor(ctr.setTrackOn[ctr.notes->at(clickNote).track]);
          }
          else{
            colorSelect.setColor(ctr.setTrackOff[ctr.notes->at(clickNote).track]);
          }
        }
        else {
          if (sheetMusicDisplay && pointInBox(GetMousePosition(), (rect){0, ctr.menuHeight, ctr.getWidth(), ctr.barHeight})) {
            selectType = SELECT_SHEET;
            colorSelect.setColor(ctr.bgSheet);
          }
          else if (pointInBox(GetMousePosition(), {int(nowLineX - 3), ctr.barHeight, 6, ctr.getHeight() - ctr.barHeight})) {
            selectType = SELECT_LINE;
            rightMenuContents[1] = "Change Line Color";
            colorSelect.setColor(ctr.bgNow);
          }
          else {
            bool measureSelected = false;
            for (unsigned int i = 0; i < ctr.file.measureMap.size(); i++) {
              double measureLineX = convertSSX(ctr.file.measureMap[i].getLocation());
              if (pointInBox(GetMousePosition(), {int(measureLineX - 3), ctr.barHeight, 6, ctr.getHeight() - ctr.barHeight}) && !menuctr.mouseOnMenu()) {
                measureSelected = true;
                break; 
              }
            }
            
            if (measureSelected) {
              selectType = SELECT_MEASURE;
              rightMenuContents[1] = "Change Line Color";
              colorSelect.setColor(ctr.bgMeasure);
            }
            else {
              selectType = SELECT_BG;
              rightMenuContents[1] = "Change Color";
              colorSelect.setColor(ctr.bgColor);
            }
          }
          rightMenu.setContent("", 0);
          auto f = rightMenuContents.begin() + 1;
          auto e = rightMenuContents.end() - 1;
          vector<string> newRight(f, e);
          rightMenu.update(newRight);
        }
        rightMenu.setXY(rightX, rightY);
        rightMenu.render = true;
      }
    }
    
    menuctr.updateMouse();
    ctr.updateKeyState();
    
    // displays index of last clicked note  
    //logQ(clickNote);
  }

  osdialog_filters_free(filetypes); 
  osdialog_filters_free(savetypes); 
  osdialog_filters_free(imagetypes); 
  UnloadFont(font);
  CloseWindow();
  return 0;
}
