#pragma once

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <raylib.h>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "asset.h"
#include "image.h"
#include "input.h"
#include "color.h"
#include "kmeans.h"
#include "output.h"
#include "colorgen.h"

using std::vector;
using std::unordered_map;
using std::stringstream;

class controller {
  public:
    controller() : midiData() {
      file = midi();
      image = imageController();
      programState = true;
      playState = false;
      livePlayState = false;
      livePlayOffset = 0;
      notes = &file.notes;
      
      getColorScheme(128, setVelocityOn, setVelocityOff);
      getColorScheme(12, setTonicOn, setTonicOff);
      getColorScheme(1, setTrackOn, setTrackOff);
    }

    void initData(vector<asset>& assets);
    void unloadData();


    Font* getFont(string id, int size);

    void updateKeyState();
    void toggleLivePlay();
    void setCloseFlag();
    void load(string path, 
                          bool& nowLine, bool& showFPS, bool& showImage, bool& sheetMusicDisplay,
                          bool& measureLine, bool& measureNumber, 

                          int& colorMode, int& displayMode,

                          int& songTimeType, int& tonicOffset, 

                          double& zoomLevel);
    void save(string path, 
                          bool nowLine, bool showFPS, bool showImage, bool sheetMusicDisplay,
                          bool measureLine, bool measureNumber, 

                          int colorMode, int displayMode,

                          int songTimeType, int tonicOffset, 

                          double zoomLevel);
    void loadTextures();
    void unloadTextures(); 

    bool getProgramState() { return programState; }
    bool getPlayState() { return playState; }
    bool getLiveState() { return livePlayState; }

    int getTrackCount();
    int getNoteCount();
    int getLastTime();
    int getTempo(int idx);

    int getWidth() { return GetScreenWidth(); }
    int getHeight() { return GetScreenHeight(); }
    point getSize() { return {GetScreenWidth(), GetScreenHeight()}; }
    point getMousePosition() { return (point){ GetMouseX(), GetMouseY()}; }

    int getSheetSize() { return getWidth() - SHEET_LMARGIN - SHEET_RMARGIN; }

    midi file;
    midiInput liveInput;
    midiOutput output;

    stringstream midiData;
    imageController image;

    vector<note>* notes;

    vector<colorRGB> setTrackOn;
    vector<colorRGB> setTrackOff;
    vector<colorRGB> setVelocityOn;
    vector<colorRGB> setVelocityOff;
    vector<colorRGB> setTonicOn;
    vector<colorRGB> setTonicOff;

    colorRGB bgDark =         colorRGB(0  , 0  , 0);
    colorRGB bgLight =        colorRGB(255, 255, 255);
    colorRGB bgNow =          colorRGB(255, 0  , 0);
    colorRGB bgMenu =         colorRGB(222, 222, 222);
    colorRGB bgMenuShade =    colorRGB(155, 155, 155);
    colorRGB bgMenuLine =     colorRGB(22 , 22 , 22);
    colorRGB bgSheet =        colorRGB(255, 252, 242);
    colorRGB bgMeasure =      colorRGB(155, 155, 155);
    colorRGB bgColor =        bgDark;

    const int menuHeight =    20;
    const int sheetHeight =   200;
    int barHeight =           0;
    int topHeight =           0;

    const int barWidth =      10;
    const int barSpacing =    80;
    const int barMargin =     49;

    int livePlayOffset;
  
    Texture2D quarter;
    Texture2D half;
    Texture2D whole;
    Texture2D flag;

    Texture2D sharp;
    Texture2D flat;
    Texture2D natural;

    Texture2D restQ;
    Texture2D restE;
    
    Texture2D treble;
    Texture2D brace;
    Texture2D bass;

    Font fontMusic;

    
    
  private:
    bool programState = true;
    bool playState;
    bool livePlayState;
  

    unordered_map<string, pair<asset, map<int, Font>>> fontMap;
    

};

