#include "controller.h"
#include <limits>
#include <stdlib.h>
#include <bitset>
#include <chrono>
#include <system_error>
#include "define.h"
#include "log.h"
#include "menuctr.h"
#include "voronoi.h"

using std::ofstream;
using std::stringstream;
using std::make_pair;

controller::controller() : midiData() {
  file = midi();
  image = imageController();
  menu = menuController();
  programState = true;
  playState = false;
  livePlayState = false;
  livePlayOffset = 0;
  notes = &file.notes;
  
  optimizeBGColor();

  getColorScheme(128, setVelocityOn, setVelocityOff);
  getColorScheme(12, setTonicOn, setTonicOff);
  getColorScheme(1, setTrackOn, setTrackOff);
}


void controller::init(vector<asset>& assetSet) {
  srand(time(0));
  
  initData(assetSet);

  text.init();

  dialog.init();

  fileOutput.init(&output);

  Vector3 startCol = {1.0f, 0.0f, 0.0f};
  setShaderValue("SH_SQUARE", "blend_color", startCol);
  

  setShaderValue("SH_VORONOI", "bg_color", bgColor2);
    
  setShaderValue("SH_FXAA", "u_resolution", (Vector2){static_cast<float>(getWidth()), static_cast<float>(getHeight())});

  voronoi.init();
  fft.updateFFTBins();

  warning.init();
}

void controller::initData(const vector<asset>& assetSet) {
  for (const auto& item : assetSet) {
    switch(item.assetType) {
      using enum ASSET;
      case FONT:
        fontMap.insert(make_pair(item.assetName, make_pair(item, map<int, Font>())));
        break;
      case IMAGE:
      {
        auto it = imageMap.find(item.assetName);
        if (it == imageMap.end()) {
          // asset not already loaded, so we should load it
          //Texture2D tmpTex = LoadTexture(item.path.c_str());
          Image tmpImg = LoadImageFromMemory(".png", item.data, item.dataLen);

          if (item.assetName == "ICON") {
            SetWindowIcon(tmpImg);
          }

          Texture2D tmpTex = LoadTextureFromImage(tmpImg);
          SetTextureFilter(tmpTex, TEXTURE_FILTER_BILINEAR);

          UnloadImage(tmpImg);

          //add to image map for future reference
          imageMap.insert(make_pair(item.assetName, tmpTex));
        }      
      }
        break;

      case SHADER: 
        {
          auto it = shaderMap.find(item.assetName);

          if (it == shaderMap.end()) {
            // asset not already loaded, so we should load it

            shaderData tmpShaderData(item);


            //add to shader map for future reference
            shaderMap.insert(make_pair(item.assetName, tmpShaderData));
          }
        }
        break;
      default:
        // these items aren't statically loaded
        break;
    }
  }

}

Font* controller::getFont(const string& id, int size) {
  // find if a font with this id exists
  auto fit = fontMap.find(id);
  if (fit == fontMap.end()) {
    logW(LL_CRIT, "invalid font id -", id);
    return nullptr;
  }

  // if this font exists, find map of font size to font pointer
  auto it = fit->second.second.find(size);
  if (it == fit->second.second.end()){ 

    // if end is reached, this font combination doesn't exist and needs to be created

    asset& tmpFontAsset = fit->second.first;

    // SMuFL defines at most 3423 (0xE000 - 0xED5F) glyphs
    logQ("loading", tmpFontAsset.assetName, "with size", size);

    int lim = 255;
    int* loc = nullptr;

    // special handling for SMuFL font
    if (tmpFontAsset.assetName == GLYPH_FONT) {
      loc = codepointSet.data();
      lim = codepointSet.size();
    }

    //logQ("font lim is", lim);
    //logQ("codepoints are", formatVector(codepointSet));

    Font tmp = LoadFontFromMemory(".otf", tmpFontAsset.data, tmpFontAsset.dataLen, size, loc , lim);

    ////logQ(sceneController->getFontData("LMP_R").assetName);

    SetTextureFilter(tmp.texture, TEXTURE_FILTER_BILINEAR);
    
    fit->second.second.insert(make_pair(size, tmp));
  
    //it = fontMap.find(size);
  }

  return &fit->second.second.find(size)->second;
  
  
}

Texture2D& controller::getImage(const string& imageIdentifier) {
  auto it = imageMap.find(imageIdentifier);
  if (it == imageMap.end()){
    logW(LL_CRIT, "attempt to load unloaded image w/ identifier: " + imageIdentifier);
    //exit(1);
  }
  return it->second; 
}

shaderData& controller::getShaderData(const string& shaderIdentifier) {
  auto it = shaderMap.find(shaderIdentifier);
  if (it == shaderMap.end()){
    logW(LL_CRIT, "attempt to load unloaded shader w/ identifier: " + shaderIdentifier);
    //exit(1);
  }
  return it->second; 
}

Shader& controller::getShader(const string& shaderIdentifier) {
  return getShaderData(shaderIdentifier).getShader();
}

void controller::unloadData() {
  for (const auto& item : fontMap) {
    for (const auto& font : item.second.second) {
      UnloadFont(font.second);
    }
  }
  for (const auto& image : imageMap) {
    UnloadTexture(image.second);
  }
  for (auto& shaderData: shaderMap) {
    shaderData.second.unloadData();
  }
  menu.unloadData();
  image.unloadData();
  
  voronoi.unloadData();

  CloseWindow();
}

void controller::beginBlendMode(int a, int b, int c) {
  rlSetBlendFactors(a, b, c);
  BeginBlendMode(BLEND_CUSTOM);
}
void controller::endBlendMode() {
  EndBlendMode();
}

void controller::processAction(actionType& action) {
  // do not overwrite pending event
  if (action != actionType::ACTION_NONE) {
    return;
  }

  // setup so that the last key in sequence
  // must be pressed last to prevent repeat inputs
  if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
    // open [file, image]
    if (IsKeyPressed(KEY_O)) {
      if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        action = actionType::ACTION_OPEN_IMAGE;
        return;
      }
      action = actionType::ACTION_OPEN;
      return;
    } 
    // close [file, image]
    if (IsKeyPressed(KEY_W)) {
      if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        action = actionType::ACTION_CLOSE_IMAGE;
        return;
      }
      action = actionType::ACTION_CLOSE;
      return;
    } 
    // save [save, save as]
    if (IsKeyPressed(KEY_S)) {
      if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        action = actionType::ACTION_SAVE_AS;
        return;
      }
      action = actionType::ACTION_SAVE;
      return;
    } 
    if (IsKeyPressed(KEY_COMMA)) {
      action = actionType::ACTION_PREFERENCES;
      return;
    }
    if (IsKeyPressed(KEY_SPACE)) {
      action = actionType::ACTION_LIVEPLAY;
      return;
    }
    if (IsKeyPressed(KEY_I)) {
      action = actionType::ACTION_INFO;
      return;
    }
  }
  
  if (IsKeyPressed(KEY_TAB)) {
    action = actionType::ACTION_PREFERENCES;
    return;
  }
  
  action = actionType::ACTION_NONE;
}

void controller::update(int offset, double& nowLineX) {
 
  if (!programState) { return; }

  frameCounter++;
  if (!livePlayState && run && output.isPortOpen()) {
      fileOutput.allow();
  }
  else {
    fileOutput.disallow();
  }
  //always update to prevent notes playing at once
  if (offset < ctr.getLastTime()) {
    fileOutput.updateOffset(offset);
  }

  menu.updateMouse();
  menu.updateRenderStatus();
  updateKeyState();
  updateDimension(nowLineX);
  updateFPS();
  curMeasure = findCurrentMeasure(offset);

  if(run) {
    runTime += GetFrameTime(); 
    pauseTime = 0; 
  }
  else {
    runTime = 0;
    pauseTime += GetFrameTime();
  }

  if (lastWidth != getWidth()) {
    lastWidth = getWidth();
  }

  if (run || livePlayState) {
    // persistent randomness for animation
    psrValue = rand();
  }

  updateDroppedFiles();
}

void controller::updateFPS() {
  if (curMon != GetCurrentMonitor()) {
    curMon = GetCurrentMonitor();
    SetTargetFPS(GetMonitorRefreshRate(curMon));
  }
}

void controller::updateKeyState() {
  if (WindowShouldClose()) {
    setCloseFlag();
  }
}

void controller::updateDimension(double& nowLineX) {
  if(IsWindowResized()) {

    setShaderValue("SH_FXAA", "u_resolution", (Vector2){static_cast<float>(getWidth()), static_cast<float>(getHeight())});


    voronoi.update();
    fft.updateFFTBins();

    nowLineX = getWidth() * nowLineX / lastWidth;
    
    if (livePlayState) {


      // TODO: measure system for live input

    }
    else {
      file.sheetData.findSheetPages();
    }
  }
}

void controller::updateFiles(char** paths, int numFile) {
  for (auto item = 0; item < numFile; ++item) {
    string ext = getExtension(paths[item]);
    if (ext == "mid" || ext == "mki") {
      if (!ctr.open_file.pending()) {
        ctr.open_file.setPending(paths[item]);
      }
    }
    else if (ext == "png" || ext == "jpg") {
      if (!ctr.open_image.pending()) {
        ctr.open_image.setPending(paths[item]);
      }
    }
    else {
      logW(LL_WARN, "invalid path:", paths[item]);
    }
  }
}

void controller::updateDroppedFiles() {
  if (IsFileDropped()) {
    FilePathList dropFile = LoadDroppedFiles();
    constexpr unsigned int dropLimit = 2;

    if (dropFile.count > dropLimit) {
      logW(LL_WARN, "excess files dropped - max:", dropLimit);
    }

    for (unsigned int idx = 0; idx < min(dropLimit, dropFile.count); ++idx) {
      updateFiles(&dropFile.paths[idx]);
    }
    UnloadDroppedFiles(dropFile); 
  }
}

void controller::toggleLivePlay() {
  if (setTrackOn.size() < 1 || option.get(optionType::OPTION_TRACK_DIVISION)) {
    getColorScheme(2, setTrackOn, setTrackOff);
  }
  livePlayState = !livePlayState;
  if (livePlayState) {
    livePlayOffset = 0;
    notes = &input.noteStream.notes;
  }
  else {
    // when turning off live input, revert to previously loaded file info
    // also, disable and clear the live input event queue
    input.resetInput();
    notes = &file.notes;
  }
  // ensure sufficient track colors
  if (file.getTrackCount() > 0) {
    getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);
  }
}

void controller::prepareCriticalSection(bool enter) {
  // if something blocks main thread for too long
  // this function notifies other threads that
  // the main thread is currently unresponsive
  // enter: true  -> prepare to enter blocking section
  // enter: false -> exit blocking section, return to normal
  if (enter) {
    run = false;
    if (getLiveState()) {
      input.pauseInput();
    }
    else {
      fileOutput.disallow();
    }
  }
  else {
    if (getLiveState()) {
      input.resumeInput();
    }
    else {
      fileOutput.allow();
    }
  }
}

void controller::optimizeBGColor(bool invert) {
  // invert exists only for when a option switch is pending, but not yet done
  bgColor2 = (invert^static_cast<bool>(option.get(optionType::OPTION_DYNAMIC_LABEL))) ? maximizeDeltaE(bgColor) : bgLight;
}

vector<string> controller::generateMenuLabels(const menuContentType& contentType) {
  switch(contentType) {
    case CONTENT_FILE:
      return text.getStringSet("FILE_MENU_FILE",
                        "FILE_MENU_OPEN_FILE",
                        "FILE_MENU_OPEN_IMAGE",
                        "FILE_MENU_SAVE",
                        "FILE_MENU_SAVE_AS",
                        "FILE_MENU_CLOSE_FILE",
                        "FILE_MENU_CLOSE_IMAGE",
                        "FILE_MENU_EXIT");
  	case CONTENT_EDIT:
			return text.getStringSet("EDIT_MENU_EDIT",
															 "EDIT_MENU_ENABLE_SHEET_MUSIC",
															 "EDIT_MENU_PREFERENCES");
  	case CONTENT_VIEW:
			return text.getStringSet("VIEW_MENU_VIEW",
															 "VIEW_MENU_DISPLAY_MODE",
															 "VIEW_MENU_DISPLAY_SONG_TIME",
															 "VIEW_MENU_HIDE_KEY_SIGNATURE",
															 "VIEW_MENU_HIDE_NOW_LINE",
                               "VIEW_MENU_HIDE_MEASURE_LINE",
															 "VIEW_MENU_HIDE_MEASURE_NUMBER",
															 "VIEW_MENU_HIDE_BACKGROUND",
															 "VIEW_MENU_SHOW_FPS");
  	case CONTENT_DISPLAY:
			return text.getStringSet("DISPLAY_MENU_DEFAULT",
															 "DISPLAY_MENU_LINE",
															 "DISPLAY_MENU_PULSE",
															 "DISPLAY_MENU_BALL",
															 "DISPLAY_MENU_FFT",
															 "DISPLAY_MENU_VORONOI",
															 "DISPLAY_MENU_LOOP");
  	case CONTENT_SONG:
			return text.getStringSet("SONG_MENU_RELATIVE",
															 "SONG_MENU_ABSOLUTE");
  	case CONTENT_MIDI:
			return text.getStringSet("MIDI_MENU_MIDI",
															 "MIDI_MENU_INPUT",
															 "MIDI_MENU_OUTPUT",
															 "MIDI_MENU_ENABLE_LIVE_PLAY");
  	case CONTENT_INPUT:
      return {""};
  	case CONTENT_OUTPUT:
      return {""};
  	case CONTENT_COLOR:
			return text.getStringSet("COLOR_MENU_COLOR",
															 "COLOR_MENU_COLOR_BY",
															 "COLOR_MENU_COLOR_SCHEME",
															 "COLOR_MENU_SWAP_COLORS",
															 "COLOR_MENU_INVERT_COLOR_SCHEME");
  	case CONTENT_SCHEME:
			return text.getStringSet("SCHEME_MENU_PART",
															 "SCHEME_MENU_VELOCITY",
															 "SCHEME_MENU_TONIC");
  	case CONTENT_INFO:
			return text.getStringSet("INFO_MENU_INFO",
															 "INFO_MENU_PROGRAM_INFO",
															 "INFO_MENU_HELP");
  	case CONTENT_PALETTE:
			return text.getStringSet("PALETTE_MENU_DEFAULT",
															 "PALETTE_MENU_FROM_BACKGROUND");
  	case CONTENT_RIGHT:
			return text.getStringSet("RIGHT_MENU_INFO",
															 "RIGHT_MENU_CHANGE_PART_COLOR",
															 "RIGHT_MENU_SET_TONIC");
  	case CONTENT_COLORSELECT:
      return text.getStringSet("COLORSELECT_COLOR_SELECT");
    default:
      logW(LL_WARN, "invalid menu label type -", contentType);
      return {};
  }
}

midi& controller::getStream() {
  if (livePlayState) {
    return input.noteStream;
  }
  return file;
}

vector<note>& controller::getNotes() {
  if (livePlayState) {
    return input.noteStream.notes;
  }
  return file.notes;
}

int controller::getTrackCount() {
  if (livePlayState) {
    return option.get(optionType::OPTION_TRACK_DIVISION) ? 2 : 1;
  }
  return file.getTrackCount();
}

int controller::getNoteCount() {
  if (livePlayState) {
    return input.getNoteCount();
  }
  return file.getNoteCount();
}

int controller::getLastTime() {
  if (livePlayState) {
    return 0;
  }
  return file.getLastTime();
}

int controller::getTempo(int idx) {
  if (livePlayState) {
    return 120;
  }
  else {
    return file.getTempo(idx);
  }
}

int controller::getMinTickLen() const {
  if (livePlayState) {
    // TODO: find/calculate default MIDI TPQ for live devices
    return 60;
  }
  else {
    return file.getMinTickLen();
  }
}

int controller::getCurrentMeasure() const {
  return curMeasure;
}

int controller::findCurrentMeasure(int pos) const {
  if (livePlayState) {
    // TODO: reimplement when measures are added to live input data
    return 0;
  }
  else {
    if (file.measureMap.size() == 0) {
      return 0;
    }
    if (pos >= (file.measureMap.end()-1)->getLocation()) {
      return file.measureMap.size();
    }
    for (unsigned int i = 0; i < file.measureMap.size(); ++i) {
      if (pos < file.measureMap[i].getLocation()-1) {
        // account for 1-based indexing of measures
        return i;
      }
    }
  }
  logW(LL_WARN, "invalid current measure at offset", pos);
  return 0;
}

int controller::getMeasureCount() const {
  if (livePlayState) {
    // TODO: reimplement when measures are added to live input data
    return 0;
  }
  else {
    return file.measureMap.size();
  }
}
    
string controller::getNoteLabel(int index) {

  string key_label = ctr.getKeySigLabel(getNotes()[index].x);

  // terrible but it works
  bool is_flat = key_label[0] == 'b';

  return getNoteInfo(getNotes()[index].track, getNotes()[index].y - MIN_NOTE_IDX, is_flat);
}

string controller::getKeySigLabel(int offset) const {

  if (livePlayState) {
    // TODO: detect key signature from already-played notes
    //return "NULL";
    return "";
  }

  if (file.measureMap.size() == 0) {
    return "";
  }

  //int KSOffset = file.measureMap[findCurrentMeasure(offset)-1].currentKey.getIndex();
  //logQ("the current measure", findCurrentMeasure(offset), "has key signature offset", KSOffset); 
  return file.measureMap[findCurrentMeasure(offset)-1].currentKey.getLabel();
}

void controller::clear() {
  midiData.clear();
  file.clear();
  runTime = 0;
  pauseTime = 0;
  bgColor = colorRGB(0,0,0);
}

void controller::load(string path, fileType& fType, 
                      bool& nowLine, bool& showFPS, bool& showImage, bool& sheetMusicDisplay,
                      bool& measureLine, bool& measureNumber, 

                      int& colorMode, int& displayMode,

                      int& songTimeType, int& tonicOffset, 

                      double& zoomLevel) {

  if (!isValidPath(path, PATH_DATA)) {
    logW(LL_WARN, "invalid path:", path);
    return;
  }

  auto start = std::chrono::high_resolution_clock::now();

  if (isValidPath(path, PATH_MKI)){
    // open input file
    ifstream input(path, std::ifstream::in | std::ios::binary);
    input.imbue(std::locale::classic());

    if(!input) {
      logW(LL_WARN, "unable to load MKI: ", path);
      return;
    }


    char byteBuf = 0;

    auto readByte = [&]() {
      if (!input.eof()) {
        input.read(&byteBuf, 1);
        return true;
      }
      logW(LL_WARN, "invalid MKI file");
      return false;
    };

    //auto debugByte = [&]() {
      //#ifndef NODEBUG

      //std::bitset<8> binRep(byteBuf);
      //logQ(binRep);


      //#endif    
    //};

    //#define readByte(); if(!readByte()) { return; }

    // 0x00 

    readByte();

    nowLine = byteBuf & (1 << 7);
    showFPS = byteBuf & (1 << 6);
    showImage = byteBuf & (1 << 5);
    sheetMusicDisplay = byteBuf & (1 << 4);
    measureLine = byteBuf & (1 << 3);
    measureNumber = byteBuf & (1 << 2);

    bool imageExists = byteBuf & (1 << 1);

    //logQ(nowLine);
    //logQ(showFPS);
    //logQ(showImage);
    //logQ(sheetMusicDisplay);
    //logQ(measureLine);
    //logQ(measureNumber);
    //logQ(imageExists);

    //debugByte();

    // 0x01-0x02 (reserved)
    readByte();
    readByte();

    
    // 0x03
    readByte();

    colorMode = (byteBuf >> 4) & 0xF;
    displayMode = byteBuf & 0xF;
    
    // 0x04
    readByte();
    //debugByte();

    songTimeType = (byteBuf >> 4) & 0xF;
    tonicOffset = byteBuf & 0x0F;

    // 0x05-0x07 (reserved)
    readByte();
    readByte();
    readByte();

    // 0x08-0x0B - zoom level
    char zoomBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      //debugByte();
      zoomBuf[i] = byteBuf;
    }

    zoomLevel = *(reinterpret_cast<float*>(zoomBuf));
    //logQ("READ ZOOMLEVEL", zoomLevel);

    //logQ(zoomLevel);
    
    // position need not exceed 16 bits
    // 0x0C-0x0D - image position (x) 
    // 0x0E-0x0F - image position (y)
    // 0x10-0x13 - scale
    // 0x14-0x17 - default scale
    // 0x18-0x1B - mean value
    // 0x1C-0x1F - color count

    // if no image exists, all values are left zero
    

    int16_t i_posx = 0;
    int16_t i_posy = 0;
    float i_scale = 0;
    float i_def_scale = 0;
    float i_mean_v = 0;
    int i_num_col = 0;

    if (imageExists) {
      
      // 0x0C-0x0D - image position (x) 
      char imageBuf[2] = {0};
      for (int i = 0; i < 2; ++i) {
        readByte();
        //debugByte();
        imageBuf[i] = byteBuf;
      }
      i_posx = *(reinterpret_cast<uint16_t*>(imageBuf));
    
      // 0x0E-0x0F - image position (y)
      //char imageBuf[4] = {0};
      for (int i = 0; i < 2; ++i) {
        readByte();
        //debugByte();
        imageBuf[i] = byteBuf;
      }
      i_posy = *(reinterpret_cast<uint16_t*>(imageBuf));

      // 0x10-0x13 - scale
      char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_scale = *(reinterpret_cast<float*>(imageBuf2));

      // 0x14-0x17 - default scale
      //char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_def_scale = *(reinterpret_cast<float*>(imageBuf2));

      // 0x18-0x1B - mean value
      //char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_mean_v = *(reinterpret_cast<float*>(imageBuf2));

      // 0x1C-0x1F - color count
      //char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_num_col = *(reinterpret_cast<uint32_t*>(imageBuf2));

    }
    else {
      // ignore image metadata block otherwise
      for (auto i = 0; i < imageBlockSize; ++i) {
        readByte();
      }
    }

    auto readRGB = [&] () {
      readByte();
      uint8_t r = *reinterpret_cast<uint8_t*>(&byteBuf);
      readByte();
      uint8_t g = *reinterpret_cast<uint8_t*>(&byteBuf);
      readByte();
      uint8_t b = *reinterpret_cast<uint8_t*>(&byteBuf);

      colorRGB col(r,g,b);

      return col;
    };

    // 0x20-0x43 - tonic (on) colors
    // 0x44-0x67 - tonic (off) colors
    for (auto& col : setTonicOn) {
      col = readRGB();
    }
    for (auto& col : setTonicOff) {
      col = readRGB();
    }
    
    // 0x68-0x1E7  velocity (on) colors
    // 0x1E8-0x367 velocity (off) colors
    for (auto& col : setVelocityOn) {
      col = readRGB();
    }
    for (auto& col : setVelocityOff) {
      col = readRGB();
    }
    
    // 0x368-0x36A - background color
    bgColor = readRGB();
    //logQ("READ COLOR", bgColor);
    
    // 0x36B-0x36E - track size marker (n)
    char trackSizeBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      //debugByte();
      trackSizeBuf[i] = byteBuf;
    }

    int trackSetSize = *(reinterpret_cast<int*>(&trackSizeBuf));
    
    if (trackSetSize > (1 << 15)) {
      logW(LL_WARN, "file parameters exceed limits");
      logW(LL_WARN, trackSetSize);
      return;
    }


    setTrackOn.resize(trackSetSize);
    setTrackOff.resize(trackSetSize);


    // 0x36F-0x36F+(n*3)       track (on) colors
    // 0x36F+(n*3)-0x36F+(n*6) track (off) colors
    for (auto i = 0; i < trackSetSize; ++i) {
      setTrackOn[i] = readRGB();
    }
    for (auto i = 0; i < trackSetSize; ++i) {
      setTrackOff[i] = readRGB();
    }

    // HERE IS MIDI STREAM DATA

    // next 4 bytes is midi size
    char midiSizeBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      //debugByte();
      midiSizeBuf[i] = byteBuf;
    }

    int midiSize = *reinterpret_cast<int*>(midiSizeBuf);
    logQ("midi size is:", midiSize);

    for (auto i = 0; i < midiSize; ++i) {
      readByte();
      midiData.write(&byteBuf, sizeof(byteBuf));
    }

    file.load(midiData);

    // HERE IS IMAGE STREAM DATA (if exists)

    if (imageExists) {

      // next 4 bytes is image format
      char imageFormatBuf[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageFormatBuf[i] = byteBuf;
      }

      // error handling of image format handled in loader function
      int imageFormat = *reinterpret_cast<int*>(&imageFormatBuf);
      //logQ("READ IMGFORMAT", imageFormat);
      
      // next 4 bytes is image size
      char imageSizeBuf[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageSizeBuf[i] = byteBuf;
      }

      int imageSize = *reinterpret_cast<int*>(&imageSizeBuf);
        
      stringstream imageData;

      for (auto i = 0; i < imageSize; ++i) {
        readByte();
        imageData.write(&byteBuf, sizeof(byteBuf));
      }

      //logQ(imageData.str());
      // finally load the image
      image.load(imageData, imageSize, imageFormat);

      // set the image parameters after loading
			image.position.x = i_posx;
			image.position.y = i_posy;
			image.scale = i_scale;
			image.defaultScale = i_def_scale;
			image.meanV = i_mean_v;
			image.numColors = i_num_col;

    }

    // update background-dependent values
    setShaderValue("SH_VORONOI", "bg_color", bgColor);
    optimizeBGColor();

    // last, set MKI loaded flag
    fType = FILE_MKI;

  }
  else {
    logW(LL_INFO, "load midi:", path);

    file.load(path, midiData);

    getColorScheme(KEY_COUNT, setVelocityOn, setVelocityOff);
    getColorScheme(TONIC_COUNT, setTonicOn, setTonicOff);
    getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);

    // last, set non-MKI loaded flag
    fType = FILE_MIDI;
  }

  fileOutput.load(file.message);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsedTime = end - start;
  logW(LL_INFO, "elapsed time:", elapsedTime.count(), "seconds");
}

void controller::save(string path, 
                      bool nowLine, bool showFPS, bool showImage, bool sheetMusicDisplay,
                      bool measureLine, bool measureNumber, 

                      int colorMode, int displayMode,

                      int songTimeType, int tonicOffset, 

                      double zoomLevel) {

  // open output file
  ofstream output(path, std::ofstream::out | std::ofstream::trunc | std::ios::binary);
  output.imbue(std::locale::classic());

  if(!output) {
    logW(LL_WARN, "unable to save file to", path);
    return;
  }

  //output << midiData.str();

  const uint8_t emptyByte = 0;

  // 0x00:[7:7] - now line 
  // 0x00:[6:6] - fps display
  // 0x00:[5:5] - image display
  // 0x00:[4:4] - sheet music display
  // 0x00:[3:3] - measure line display
  // 0x00:[2:2] - measure number display
  // 0x00:[1:1] - image existence
  // 0x00:[0:0] - reserved

  uint8_t byte0 = 0;
  byte0 |= (nowLine << 7);
  byte0 |= (showFPS << 6);
  byte0 |= (showImage << 5);
  byte0 |= (sheetMusicDisplay << 4);
  byte0 |= (measureLine << 3);
  byte0 |= (measureNumber << 2);
  byte0 |= (image.exists() << 1);

  //logQ(byte0);

  output.write(reinterpret_cast<const char*>(&byte0), sizeof(byte0));

  
  // 0x01:[7:0] - reserved
  // 0x02:[7:0] - reserved

  output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
  output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
  
  // 0x03:[7:4] - scheme color type
  // 0x03:[3:0] - display type
  
  uint8_t byte3 = 0;

  byte3 |= (static_cast<uint8_t>(colorMode) << 4);
  byte3 |= (static_cast<uint8_t>(displayMode) & 0xF);

  //logQ((unsigned int)byte3);

  output.write(reinterpret_cast<const char*>(&byte3), sizeof(byte3));
  
  // 0x04:[7:4] - song time display type
  // 0x04:[3:0] - tonic offset
  
  uint8_t byte4 = 0;

  byte4 |= (static_cast<uint8_t>(songTimeType) << 4);
  //logQ(bitset<8>(songTimeType));
  //logQ(bitset<8>(byte4));
  byte4 |= (static_cast<uint8_t>(tonicOffset) & 0xF);

  output.write(reinterpret_cast<const char*>(&byte4), sizeof(byte4));
  
  // 0x05:[7:0] - reserved
  // 0x06:[7:0] - reserved
  // 0x07:[7:0] - reserved
  
  output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
  output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
  output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
  
  // 0x08-0x0B - zoom level (4bit float (cast from double))
 
  float zlf = static_cast<float>(zoomLevel);

  //logQ("WRITE ZOOMLEVEL", zoomLevel);


  //logQ("zl", zlf);

  output.write(reinterpret_cast<const char*>(&zlf), sizeof(zlf));
  //logQ(static_cast<float>(zoomLevel));

  // position need not exceed 16 bits
  // 0x0C-0x0D - image position (x) 
  // 0x0E-0x0F - image position (y)
  // 0x10-0x13 - scale
  // 0x14-0x17 - default scale
  // 0x18-0x1B - mean value
  // 0x1C-0x1F - color count

  // if no image exists, all values are left zero

  if (image.exists()) {
    int16_t x = static_cast<int16_t>(image.position.x);
    int16_t y = static_cast<int16_t>(image.position.y);
    output.write(reinterpret_cast<const char*>(&x), sizeof(y));
    output.write(reinterpret_cast<const char*>(&y), sizeof(y));

    float i_scale = image.scale;
    float i_defaultScale = image.defaultScale;
    float i_meanV = image.meanV;

    output.write(reinterpret_cast<const char*>(&i_scale), sizeof(i_scale));
    output.write(reinterpret_cast<const char*>(&i_defaultScale), sizeof(i_defaultScale));
    output.write(reinterpret_cast<const char*>(&i_meanV), sizeof(i_meanV));
    output.write(reinterpret_cast<const char*>(&image.numColors), sizeof(image.numColors));
  }
  else {
   
    // needed to maintain file block formatting

    for (auto i = 0; i < imageBlockSize; ++i) {
      output.write(reinterpret_cast<const char*>(&emptyByte), sizeof(emptyByte));
    }
  }

  // colorRGB has 3 bytes per object
  // track order (on, off):
  // tonic(12)            -> 72    bytes of tonic color data 
  // velocity(128)        -> 768   bytes of velocity color data
  // track(variable)      -> n*3*2 bytes of track color data

  auto writeRGB = [&] (colorRGB col) {

    uint8_t r = round(col.r);
    uint8_t g = round(col.g);
    uint8_t b = round(col.b);

    // all items are uint8_t
    output.write(reinterpret_cast<const char*>(&r), sizeof(r));
    output.write(reinterpret_cast<const char*>(&g), sizeof(g));
    output.write(reinterpret_cast<const char*>(&b), sizeof(b));
  };

  // 0x20-0x43 - tonic (on) colors
  // 0x44-0x67 - tonic (off) colors
  for (auto col : setTonicOn) {
    writeRGB(col);
  }
  for (auto col : setTonicOff) {
    writeRGB(col);
  }

  // 0x68-0x1E7  velocity (on) colors
  // 0x1E8-0x367 velocity (off) colors
  for (auto col : setVelocityOn) {
    writeRGB(col);
  }
  for (auto col : setVelocityOff) {
    writeRGB(col);
  }
 
  // 0x368-0x36A - background color
  writeRGB(bgColor);
  //logQ("WRITE COLOR", bgColor);
  
  // 0x36B-0x36E - track size marker (n)
  // 0x36F-0x36F+(n*3)       track (on) colors
  // 0x36F+(n*3)-0x36F+(n*6) track (off) colors
  uint32_t trackSetSize = setTrackOn.size();
  //logQ("OUTN", trackSetSize); 
  output.write(reinterpret_cast<const char*>(&trackSetSize), sizeof(trackSetSize));

  for (auto col : setTrackOn) {
    writeRGB(col);
  }
  for (auto col : setTrackOff) {
    writeRGB(col);
  }

  //logQ(midiData.str());

  // get size of midi data
  //midiData.seekg(0, std::ios::end);
  //uint32_t midiSize = midiData.tellg();
  //midiData.seekg(0, std::ios::beg);
  uint32_t midiSize = midiData.str().size();
 
  logQ("midisize marker is", sizeof(midiSize), "bytes");
  output.write(reinterpret_cast<const char*>(&midiSize), sizeof(midiSize));
  logQ("saved midi stream length is", midiSize, "bytes");
  output.write(midiData.str().c_str(), midiData.str().size());


  // in variable length regime, only write if the marker is set at 0x00[1:1] 
  // here, no need to check marker, just check for existence of a loaded image
  if (image.exists()) {
    // first write image type (4bytes)
    output.write(reinterpret_cast<const char*>(&image.format), sizeof(image.format));

    //logQ("WRITE IMGFORMAT", image.format);
    
    // get size of image data
    //image.buf.seekg(0, std::ios::end);
    //uint32_t imageSize = image.buf.tellg();
    //image.buf.seekg(0, std::ios::beg);
    uint32_t imageSize = image.buf.str().size();
    
    output.write(reinterpret_cast<const char*>(&imageSize), sizeof(imageSize));
    //logQ(imageSize);
    //output.write(image.buf.str().c_str(), image.buf.str().size());
    output << image.buf.rdbuf();

  //logQ(image.buf.str());
  }

}

void controller::setCloseFlag() {
  programState = false;
  fileOutput.terminate();
}
