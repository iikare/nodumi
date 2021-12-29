#pragma once

#include <vector>
#include <functional>
#include <raylib.h>
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "image.h"
#include "input.h"
#include "color.h"
#include "output.h"
#include "colorgen.h"

using std::vector;

class controller {
  public:
    controller() {
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

    void updateKeyState();
    void toggleLivePlay();
    void setCloseFlag();
    void load(string filename);
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

    imageController image;

    vector<note>* notes;

    vector<colorRGB> setTrackOn;
    vector<colorRGB> setTrackOff;
    vector<colorRGB> setVelocityOn;
    vector<colorRGB> setVelocityOff;
    vector<colorRGB> setTonicOn;
    vector<colorRGB> setTonicOff;

    colorRGB bgDark = colorRGB(0, 0, 0);
    colorRGB bgLight = colorRGB(255, 255, 255);
    colorRGB bgNow = colorRGB(255, 0, 0);
    colorRGB bgMenu = colorRGB(222, 222, 222);
    colorRGB bgMenuShade = colorRGB(155, 155, 155);
    colorRGB bgMenuLine = colorRGB(22, 22, 22);
    colorRGB bgColor = bgDark;
    colorRGB bgSheet = colorRGB(255, 252, 242);
    colorRGB bgMeasure = colorRGB(155, 155, 155);

    const int menuHeight = 20;
    const int sheetHeight = 200;
    int barHeight = 0;

    const int barWidth = 10;
    const int barSpacing = 80;
    const int barMargin = 49;

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
    

};

