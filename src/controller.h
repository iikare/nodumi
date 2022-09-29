#pragma once

#if defined(LOCRAY)
  #include "../dpd/raylib/src/raylib.h"
#else
  #include <raylib.h>
#endif

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
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

    void update(int offset, bool runState);
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

    bool getProgramState() { return programState; }
    bool getPlayState() { return playState; }
    bool getLiveState() { return livePlayState; }
    double getRunTime() { return runTime; }
    double getPauseTime() { return pauseTime; }

    vector<note>* getNotes();
    int getTrackCount();
    int getNoteCount();
    int getLastTime();
    int getTempo(int idx);
    int getMinTickLen() const;
    int getCurrentMeasure() const;
    int getMeasureCount() const;

    int getWidth() const { return GetScreenWidth(); }
    int getHeight() const { return GetScreenHeight(); }
    point getSize() const { return {GetScreenWidth(), GetScreenHeight()}; }
    point getMousePosition() const; 
    int getMouseX() const { return getMousePosition().x; }; 
    int getMouseY() const { return getMousePosition().y; }; 
    
    int getSheetSize() const { return getWidth() - SHEET_LMARGIN - SHEET_RMARGIN; }

    midi file;
    midiInput liveInput;
    midiOutput output;

    stringstream midiData;
    imageController image;


    vector<colorRGB> setTrackOn;
    vector<colorRGB> setTrackOff;
    vector<colorRGB> setVelocityOn;
    vector<colorRGB> setVelocityOff;
    vector<colorRGB> setTonicOn;
    vector<colorRGB> setTonicOff;

    colorRGB bgDark =         colorRGB(0  , 0  , 0  );
    colorRGB bgLight =        colorRGB(255, 255, 255);
    colorRGB bgNow =          colorRGB(255, 0  , 0  );
    colorRGB bgColor =        bgDark;
    colorRGB bgMenu =         colorRGB(222, 222, 222);
    colorRGB bgMenuShade =    colorRGB(155, 155, 155);
    colorRGB bgMenuLine =     colorRGB(22 , 22 , 22 );
    colorRGB bgSheet =        colorRGB(255, 252, 242);
    colorRGB bgSheetNote =    colorRGB(0  , 0  , 0  );
    colorRGB bgMeasure =      colorRGB(155, 155, 155);

    static constexpr int menuHeight  =   20;
    static constexpr int sheetHeight =   250;
    int barHeight =                      0;
    int topHeight =                      0;

    static constexpr int barWidth    =   10;
    static constexpr int barSpacing  =   80;
    static constexpr int barMargin   =   49 + (sheetHeight-200)/2;

    static constexpr int sheetSideMargin = 30;
    static constexpr int sheetSymbolWidth = sheetSideMargin + 42;

    double livePlayOffset;
    int curMeasure;
    
    
    
  private:
    void updateKeyState();
    void updateDimension();
    void updateFPS();
    int findCurrentMeasure(int pos) const;
    
    bool programState = true;
    bool playState;
    bool livePlayState;
 
    int curMon;
    double runTime;
    double pauseTime;
    
    vector<note>* notes;


    unordered_map<string, pair<asset, map<int, Font>>> fontMap;
    

};

