#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <raylib.h>
#include "box.h"
#include "misc.h"
#include "menu.h"
#include "data.h"
#include "wrap.h"
#include "color.h"
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

  /*
   * * * * * 
   * SETUP *
   * * * * *
   */ 
  
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(mWidth, mHeight, (string("kelumi ") + string(mVersion)).c_str());
  SetTargetFPS(60);
  font = LoadFontEx("bin/fonts/yklight.ttf", 14, 0, 250);
  
  /*
   * * * * * * * 
   * VARIABLES *
   * * * * * * *
   */  
  

  // scaling settings
  double zoomLevel = 0.125;
  double timeOffset = 0;
  const double shiftC = 2;

  // play settings
  bool run = false;

  // view settings
  bool nowLine = true;
  bool colorMove = false;
  bool colorSquare = false;
  bool colorCircle = false;
  int songTimeType = 0;
  bool showFPS = false;

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

  // menu objects
  vector<string> fileMenuContents = {"File", "Open File", "Open Image", "Save", "Save As", "Exit"};
  menu fileMenu(ctr.getSize(), fileMenuContents, nullptr, TYPE_MAIN, menuctr.getOffset(), 0);
  menuctr.registerMenu(&fileMenu);
   
  vector<string> editMenuContents = {"Edit", "Preferences"};
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
      logII(LL_WARN, "invalid filename: " + filename);
    }
    filecheck.close();

    ctr.load(filename);
  }

  while (ctr.getProgramState()) {

    if (newFile) {
      newFile = false;

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
      if (nowLine) {
        if (pointInBox(GetMousePosition(), {ctr.getWidth()/2 - 3, ITEM_HEIGHT, 6, ctr.getHeight() - ITEM_HEIGHT}) &&
            !menuctr.mouseOnMenu()) {
            drawLineEx(ctr.getWidth()/2, ITEM_HEIGHT, ctr.getWidth()/2, ctr.getHeight(), 1, ctr.bgNow);
        }
        else {
          drawLineEx(ctr.getWidth()/2, ITEM_HEIGHT, ctr.getWidth()/2, ctr.getHeight(), 0.5, ctr.bgNow);
        }
      }
      for (int i = 0; i < ctr.getNoteCount(); i++) {

        double cX = ctr.getWidth() / 2 + (ctr.notes->at(i).x - timeOffset) * zoomLevel;
        double cY = (ctr.getHeight() - (ctr.getHeight() - ctr.menuHeight) * 
                  static_cast<double>(ctr.notes->at(i).y - MIN_NOTE_IDX + 3)/(NOTE_RANGE + 3));
        double cW = ctr.notes->at(i).duration * zoomLevel < 1 ? 1 : ctr.notes->at(i).duration * zoomLevel;
        double cH = (ctr.getHeight() - ctr.menuHeight) / 88;
        
        int colorID = 0;

        switch (colorMode) {
          case COLOR_PART:
            colorID = ctr.notes->at(i).track;
            break;
          case COLOR_VELOCITY:
            colorID = ctr.notes->at(i).velocity;
            break;
          case COLOR_TONIC:
            colorID = (ctr.notes->at(i).y - MIN_NOTE_IDX) % 12;
            break;
        }

        if (cX + cW > 0 && cX < ctr.getWidth()) {
            
          bool noteOn = false;

          if (ctr.notes->at(i).isOn ||
             (timeOffset >= ctr.notes->at(i).x && timeOffset < ctr.notes->at(i).x + ctr.notes->at(i).duration)) {
            noteOn = true;
          }
          if (pointInBox(GetMousePosition(), (rect){int(cX), int(cY), int(cW), int(cH)}) && !menuctr.mouseOnMenu()) {
            noteOn ? clickOnTmp = true : clickOnTmp = false;
            noteOn = !noteOn;
            clickTmp = i;
          }
          
          if (noteOn) {
            drawRectangle(cX, cY, cW, cH, colorSetOn->at(colorID));
          }
          else {
            drawRectangle(cX, cY, cW, cH, colorSetOff->at(colorID));
          }
        }
      }

      // menu bar rendering
      drawRectangle(0, 0, ctr.getWidth(), ITEM_HEIGHT, ctr.bgMenu);  

      // option actions
      if (songTimeType == 1) {
          drawTextEx(font, getSongPercent(timeOffset, ctr.getLastTick()).c_str(), 6, 26, ctr.bgLight);
      }
      else if (songTimeType == 2) {
          drawTextEx(font, 
         getSongTime(timeOffset, ctr.getLastTick()).c_str(), 6, 26, ctr.bgLight);
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
      if (timeOffset + GetFrameTime() * 500< ctr.getLastTick()) {
        timeOffset += GetFrameTime() * 500;
      }
      else {
        timeOffset = ctr.getLastTick();
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
      if (clickNote != -1) {
        if (clickOn) {
          ctr.setTrackOn[ctr.notes->at(clickNote).track] = colorSelect.getColor();
        }
        else {
          ctr.setTrackOff[ctr.notes->at(clickNote).track] = colorSelect.getColor();
        }
      }
      else if (selectType == SELECT_BG) {
        ctr.bgColor = colorSelect.getColor();
      }
      else if (selectType == SELECT_LINE){
        ctr.bgNow = colorSelect.getColor();
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
        if (timeOffset + shiftC * 60 < ctr.getLastTick()) {
          timeOffset += shiftC * 60;
        }
        else if (timeOffset < ctr.getLastTick()) {
          timeOffset = ctr.getLastTick();
        }
      }
      else {
        if (timeOffset + shiftC * 6 < ctr.getLastTick()) {
          timeOffset += shiftC * 6;
        }
        else {
          timeOffset = ctr.getLastTick();
        }
      }
    }
    if (IsKeyDown(KEY_HOME)) {
      timeOffset = 0;
    }
    if (IsKeyDown(KEY_END)) {
      timeOffset = ctr.getLastTick();
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
              songTimeType = 1;
              viewMenu.setContent("Hide Song Time", 2);
              break;
            case 1:
              songTimeType = 2;
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
                  songTimeType = 0;
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
              if (rightMenu.getSize() == 2) {
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
          rightX = round(ctr.getWidth() / 2 + (ctr.notes->at(clickNote).x - timeOffset) * zoomLevel);
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
          if (pointInBox(GetMousePosition(), {ctr.getWidth()/2 - 3, ITEM_HEIGHT, 6, ctr.getHeight() - ITEM_HEIGHT})) {
            selectType = SELECT_LINE;
            rightMenuContents[1] = "Change Line Color";
            colorSelect.setColor(ctr.bgNow);
          }
          else {
            selectType = SELECT_BG;
            rightMenuContents[1] = "Change Color";
            colorSelect.setColor(ctr.bgColor);
          }
          rightMenu.setContent("", 0);
          auto f = rightMenuContents.begin() + 1;
          auto e = rightMenuContents.end();
          vector<string> newRight(f, e);
          rightMenu.update(newRight);
        }
        rightMenu.setXY(rightX, rightY);
        rightMenu.render = true;
      }
    }

    ctr.updateKeyState();
  }

  osdialog_filters_free(filetypes); 
  osdialog_filters_free(savetypes); 
  osdialog_filters_free(imagetypes); 
  UnloadFont(font);
  CloseWindow();
  return 0;
}
