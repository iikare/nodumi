#pragma once

#include "build_target.h"

#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
#include "io.h"
#include "fft.h"
#include "midi.h"
#include "misc.h"
#include "data.h"
#include "text.h"
#include "asset.h"
#include "image.h"
#include "input.h"
#include "color.h"
#include "kmeans.h"
#include "output.h"
#include "shader.h"
#include "option.h"
#include "dialog.h"
#include "warning.h"
#include "voronoi.h"
#include "colorgen.h"
#include "output_sync.h"

using std::vector;
using std::unordered_map;
using std::stringstream;

class menuController;
class voronoiController;

class controller {
  public:

    controller();
    ~controller();

    void init(vector<asset>& assetSet);
    void unloadData();
    void setCloseFlag();

    Font* getFont(const string& id, int size);
    Texture2D& getImage(const string& imageIdentifier);
    Shader& getShader(const string& shaderIdentifier);

    void beginShaderMode(const string& shaderIdentifier) { BeginShaderMode(getShader(shaderIdentifier)); }
    void endShaderMode() { EndShaderMode(); }

    void beginBlendMode(int a, int b, int c);
    void endBlendMode();

    void beginTextureMode(const RenderTexture& rtex) { BeginTextureMode(rtex); }
    void endTextureMode() { EndTextureMode(); }

    template <class T>
    void setShaderValue(const string& shader, const string& uf, const T& val, const int num = -1) {
      getShaderData(shader).setShaderValue(uf, val, num);
    }

    void toggleLivePlay();
    void prepareCriticalSection(bool enter);

    void update(int offset, double& nowLineX, bool runState);
    void updateFiles(char** paths, int numFile = 1);
    void processAction(actionType& action);


    vector<string> generateMenuLabels(const menuContentType& contentType);

    void clear();
    void load(string path, fileType& fType, 
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
    int getPSR() { return psrValue; }
    unsigned int getFrameCounter() { return frameCounter; }
    
    midi& getStream();
    vector<note>& getNotes();
    int getTrackCount();
    int getNoteCount();
    int getLastTime();
    int getTempo(int idx);
    int getMinTickLen() const;
    int getCurrentMeasure() const;
    int getMeasureCount() const;
    string getKeySigLabel(int offset) const;

    int getWidth() const { return GetScreenWidth(); }
    int getHeight() const { return GetScreenHeight(); }
    point getSize() const { return {GetScreenWidth(), GetScreenHeight()}; }
    int getMouseX() const { return getMousePosition().x; }; 
    int getMouseY() const { return getMousePosition().y; }; 
    
    int getSheetSize() const { return getWidth() - SHEET_LMARGIN - SHEET_RMARGIN; }

    midi file;
    midiInput input;
    midiOutput output;
    stringstream midiData;

    textController text;
    imageController image;
    menuController* menu;
    dialogController dialog;
    optionController option;
    ioController open_file  = ioController(OSDIALOG_OPEN, FILTER_FILE);
    ioController open_image = ioController(OSDIALOG_OPEN, FILTER_IMAGE);
    ioController save_file  = ioController(OSDIALOG_SAVE, FILTER_SAVE);
    warningController warning;

    voronoiController voronoi;
    fftController fft;

    bool run = false;

    vector<colorRGB> setTrackOn;
    vector<colorRGB> setTrackOff;
    vector<colorRGB> setVelocityOn;
    vector<colorRGB> setVelocityOff;
    vector<colorRGB> setTonicOn;
    vector<colorRGB> setTonicOff;

    colorRGB bgDark         = colorRGB(0  , 0  , 0  );
    colorRGB bgLight        = colorRGB(255, 255, 255);
    colorRGB bgNow          = colorRGB(255, 0  , 0  );
    colorRGB bgColor        = bgDark;
    colorRGB bgMenu         = colorRGB(222, 222, 222);
    colorRGB bgMenuShade    = colorRGB(155, 155, 155);
    colorRGB bgMenuLine     = colorRGB(22 , 22 , 22 );
    colorRGB bgSheet        = colorRGB(255, 252, 242);
    colorRGB bgSheetNote    = colorRGB(0  , 0  , 0  );
    colorRGB bgMeasure      = colorRGB(155, 155, 155);
    colorRGB bgIcon         = colorRGB(34 , 115, 150);
    colorRGB bgOpt          = colorRGB(215, 38 , 61 );
    colorRGB bgOpt2         = colorRGB(56 , 145, 166);

    static constexpr int menuHeight  =   20;
    static constexpr int sheetHeight =   250;
    int barHeight =                      0;
    int topHeight =                      0;

    static constexpr int barWidth    =   10;
    static constexpr int barSpacing  =   80;
    static constexpr int barMargin   =   49 + (sheetHeight-200)/2;

    static constexpr int sheetSideMargin = 30;
    static constexpr int sheetSymbolWidth = sheetSideMargin + 42;

    static constexpr int prefWidth   =   700;
    static constexpr int prefHeight  =   500;

    static constexpr int infoWidth   =   500;
    static constexpr int infoHeight  =   268;

    // shared across load/save routines
    static constexpr int imageBlockSize = 20;

    double livePlayOffset;
    int curMeasure;


    friend class ioController;

  private:
    void initData(const vector<asset>& assetSet);

    void updateKeyState();
    void updateDimension(double& nowLineX);
    void updateFPS();
    void updateDroppedFiles();
    
    int findCurrentMeasure(int pos) const;


    
    shaderData& getShaderData(const string& shaderIdentifier);
    
    int lastWidth = 0;

    bool programState = true;
    bool playState;
    bool livePlayState;

    unsigned int frameCounter = 0;
    int curMon;
    double runTime;
    double pauseTime;
    
    int psrValue = 0;

    vector<note>* notes;

    outputInstance fileOutput;



    unordered_map<string, pair<asset, map<int, Font>>> fontMap;
    unordered_map <string, Texture2D> imageMap;
    unordered_map <string, shaderData> shaderMap;
    

};

