#pragma once

#include <functional>
#include <random>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "asset.h"
#include "buffer.h"
#include "build_target.h"
#include "color.h"
#include "colorgen.h"
#include "data.h"
#include "dialog.h"
#include "fft.h"
#include "image.h"
#include "input.h"
#include "io.h"
#include "kmeans.h"
#include "menuctr.h"
#include "midi.h"
#include "misc.h"
#include "option.h"
#include "output.h"
#include "output_sync.h"
#include "particle.h"
#include "shader.h"
#include "shadow.h"
#include "text.h"
#include "voronoi.h"
#include "warning.h"

using std::stringstream;
using std::unordered_map;
using std::vector;

class menuController;
class voronoiController;

class controller {
 public:
  controller();

  void init(vector<asset>& assetSet);
  void unloadData();
  void setCloseFlag();

  const Font& getFont(const string& id, int size);
  Texture2D& getImage(const string& imageIdentifier);
  Shader& getShader(const string& shaderIdentifier);

  void beginFrame();
  void endFrame();

  void beginShaderMode(const string& shaderIdentifier) { BeginShaderMode(getShader(shaderIdentifier)); }
  void endShaderMode() { EndShaderMode(); }

  void beginBlendMode(int a) { BeginBlendMode(a); }
  void beginBlendMode(int a, int b, int c);
  void endBlendMode();

  void beginTextureMode(const RenderTexture& rtex) { BeginTextureMode(rtex); }
  void endTextureMode() { EndTextureMode(); }

  template <class T>
  void setShaderValue(const string& shader, const string& uf, const T& val, const int num = -1) {
    getShaderData(shader).setShaderValue(uf, val, num);
  }

  void updateFrameBuffer();

  void toggleLivePlay();
  void criticalSection(bool enter);

  void update(int offset, double zoom, double& nowLineX);
  void updateFiles(char** paths, int numFile = 1);
  ACTION process(ACTION action);

  void updateFPSCap(bool state);

  void optimizeBGColor(bool invert = false);
  vector<colorRGB>& findColorSet(int colorMode, bool on);

  vector<string> generateMenuLabels(const menuContentType& contentType);

  void clear();
  void load(string path, bool& nowLine, bool& showFPS, bool& showImage, bool& sheetMusicDisplay, bool& measureLine,
            bool& measureNumber,

            int& colorMode, int& displayMode,

            int& songTimeType, int& tonicOffset,

            double& zoomLevel);
  void save(string path, bool nowLine, bool showFPS, bool showImage, bool sheetMusicDisplay, bool measureLine,
            bool measureNumber,

            int colorMode, int displayMode,

            int songTimeType, int tonicOffset,

            double zoomLevel);

  bool getProgramState() const { return programState; }
  bool getPlayState() const { return playState; }
  bool getLiveState() const { return livePlayState; }
  fileType getFileType() const;
  string getFilePath() const;
  string getFileFullPath() const;
  double getRunTime() const { return runTime; }
  double getPauseTime() const { return pauseTime; }
  int getPSR() const { return psrValue; }
  int getRandRange(int a, int b);
  double getRandClamp();
  unsigned int getFrameCounter() const { return frameCounter; }

  midi& getStream();
  vector<note>& getNotes();
  int getTrackCount();
  int getNoteCount();
  int getLastTime();
  int getTempo(int idx) const;
  int getMinTickLen() const;
  int getCurrentMeasure() const;
  int getMeasureCount() const;
  string getKeySigLabel(int offset) const;
  string getTempoLabel(int offset) const;
  string getNoteLabel(int index);

  int getWidth() const { return GetScreenWidth(); }
  int getHeight() const { return GetScreenHeight(); }
  point getSize() const { return {GetScreenWidth(), GetScreenHeight()}; }
  int getMouseX() const { return getMousePosition().x; };
  int getMouseY() const { return getMousePosition().y; };

  int getSheetSize() const { return getWidth() - SHEET_LMARGIN - SHEET_RMARGIN; }

  int findCurrentMeasure(int pos) const;

  midi file;
  midiInput input;
  midiOutput output;
  stringstream midiData;

  textController text;
  imageController image;
  menuController menu;
  dialogController dialog;
  optionController option;
  ioController open_file = ioController(OSDIALOG_OPEN, FILTER_FILE);
  ioController open_image = ioController(OSDIALOG_OPEN, FILTER_IMAGE);
  ioController save_file = ioController(OSDIALOG_SAVE, FILTER_SAVE);
  warningController warning;
  bufferController buffer;

  particleController particle;
  shadowController shadow;
  voronoiController voronoi;
  fftController fft;

  bool run = false;

  vector<colorRGB> setTrackOn;
  vector<colorRGB> setTrackOff;
  vector<colorRGB> setVelocityOn;
  vector<colorRGB> setVelocityOff;
  vector<colorRGB> setTonicOn;
  vector<colorRGB> setTonicOff;

  colorRGB bgDark = colorRGB(0, 0, 0);
  colorRGB bgLight = colorRGB(255, 255, 255);
  colorRGB bgNow = colorRGB(255, 0, 0);
  colorRGB bgColor = bgDark;
  colorRGB bgColor2 = bgLight;
  colorRGB bgMenu = colorRGB(222, 222, 222);
  colorRGB bgMenuShade = colorRGB(155, 155, 155);
  colorRGB bgMenuLine = colorRGB(22, 22, 22);
  colorRGB bgSheet = colorRGB(255, 252, 242);
  colorRGB bgSheetNote = colorRGB(0, 0, 0);
  colorRGB bgMeasure = colorRGB(155, 155, 155);
  colorRGB bgIcon = colorRGB(34, 115, 150);
  colorRGB bgOpt = colorRGB(215, 38, 61);
  colorRGB bgOpt2 = colorRGB(56, 145, 166);
  colorRGB bgWhite = colorRGB(255, 255, 255);

  static constexpr int menuHeight = 20;
  static constexpr int sheetHeight = 250;
  int barHeight = 0;
  int topHeight = 0;

  static constexpr int barWidth = 10;
  static constexpr int barSpacing = 80;
  static constexpr int barMargin = 49 + (sheetHeight - 200) / 2;

  static constexpr int sheetSideMargin = 30;
  static constexpr int sheetSymbolWidth = sheetSideMargin + 42;

  static constexpr int prefWidth = 500;
  static constexpr int prefHeight = 254;

  static constexpr int fileWidth = 500;
  static constexpr int fileHeight = 254;

  static constexpr int infoWidth = 500;
  static constexpr int infoHeight = 254;

  // shared across load/save routines
  static constexpr int imageBlockSize = 20;

  double livePlayOffset;
  int curMeasure;
  int pendingActionValue;
  string pendingColorValue;

  friend class ioController;

 private:
  void initData(const vector<asset>& assetSet);

  void updateKeyState();
  void updateDimension(double& nowLineX);
  void updateFPS(bool bypass = false);
  void updateDroppedFiles();

  RenderTexture framebuffer;
  shaderData& getShaderData(const string& shaderIdentifier);

  int lastWidth = 0;

  bool programState = true;
  bool playState;
  bool livePlayState;
  fileType fType;
  string fPath;

  unsigned int frameCounter = 0;
  int curMon;
  bool uncapped_fps = false;
  double runTime;
  double pauseTime;

  int psrValue = 0;
  std::random_device rd;
  std::mt19937 gen;

  vector<note>* notes;

  outputInstance fileOutput;

  unordered_map<string, pair<asset, map<int, Font>>> fontMap;
  unordered_map<string, Texture2D> imageMap;
  unordered_map<string, shaderData> shaderMap;
};
