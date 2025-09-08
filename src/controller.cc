#include "controller.h"

#include <stdlib.h>

#include <bitset>
#include <chrono>
#include <filesystem>
#include <limits>
#include <random>

#include "build_target.h"
#include "define.h"
#include "log.h"
#include "menuctr.h"
#include "voronoi.h"
#include "wrap.h"

using std::make_pair;
using std::ofstream;
using std::stringstream;

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

  std::mt19937 gen(rd());

  initData(assetSet);

  SetWindowMinSize(W_WIDTH, W_HEIGHT);

  text.init();

  dialog.init();

  fileOutput.init(&output);

  updateFrameBuffer();

  Vector3 startCol = {1.0f, 0.0f, 0.0f};
  setShaderValue("SH_SQUARE", "blend_color", startCol);

  setShaderValue("SH_VORONOI", "bg_color", bgColor2);

  setShaderValue("SH_LEVEL", "use_bw", 0);
  setShaderValue("SH_LEVEL", "use_rgb", 0);
  setShaderValue("SH_LEVEL", "thr_bw", 1.0f);
  setShaderValue("SH_LEVEL", "thr_r", 1.0f);
  setShaderValue("SH_LEVEL", "thr_g", 1.0f);
  setShaderValue("SH_LEVEL", "thr_b", 1.0f);

  updateResolution();
  shadow.init();
  voronoi.init();
  fft.updateFFTBins();

  warning.init();

  updateFPS(true);
}

void controller::initData(const vector<asset>& assetSet) {
  for (const auto& item : assetSet) {
    switch (item.assetType) {
      using enum ASSET;
      case FONT:
        fontMap.insert(make_pair(item.assetName, make_pair(item, map<int, Font>())));
        break;
      case IMAGE: {
        auto it = imageMap.find(item.assetName);
        if (it == imageMap.end()) {
          // asset not already loaded, so we should load it
          // Texture2D tmpTex = LoadTexture(item.path.c_str());
          Image tmpImg = LoadImageFromMemory(".png", item.data, item.dataLen);

          if (item.assetName == "ICON") {
            SetWindowIcon(tmpImg);
          }

          Texture2D tmpTex = LoadTextureFromImage(tmpImg);
          SetTextureFilter(tmpTex, TEXTURE_FILTER_BILINEAR);

          UnloadImage(tmpImg);

          // add to image map for future reference
          imageMap.insert(make_pair(item.assetName, tmpTex));
        }
      } break;

      case SHADER: {
        auto it = shaderMap.find(item.assetName);

        if (it == shaderMap.end()) {
          // asset not already loaded, so we should load it

          shaderData tmpShaderData(item);

          // add to shader map for future reference
          shaderMap.insert(make_pair(item.assetName, tmpShaderData));
        }
      } break;
      default:
        // these items aren't statically loaded
        break;
    }
  }
}

const Font& controller::getFont(const string& id, int size) {
  // find if a font with this id exists
  auto fit = fontMap.find(id);
  if (fit == fontMap.end()) {
    logW(LL_CRIT, "invalid font id -", id);
    // return nullptr;
    fit = fontMap.begin();
  }

  // if this font exists, find map of font size to font pointer
  auto it = fit->second.second.find(size);
  if (it == fit->second.second.end()) {
    // if end is reached, this font combination doesn't exist and needs to be
    // created

    asset& tmpFontAsset = fit->second.first;

    // SMuFL defines at most 3423 (0xE000 - 0xED5F) glyphs
    // logQ("loading", tmpFontAsset.assetName, "with size", size);

    int lim = 255;
    int* loc = nullptr;

    // special handling for SMuFL font
    if (tmpFontAsset.assetName == GLYPH_FONT) {
      loc = codepointSet.data();
      lim = codepointSet.size();
    }

    // logQ("font lim is", lim);
    // logQ("codepoints are", formatVector(codepointSet));

    Font tmp = LoadFontFromMemory(".otf", tmpFontAsset.data, tmpFontAsset.dataLen, size, loc, lim);

    SetTextureFilter(tmp.texture, TEXTURE_FILTER_BILINEAR);

    fit->second.second.insert(make_pair(size, tmp));

    // it = fontMap.find(size);
  }

  return fit->second.second.find(size)->second;
}

Texture2D& controller::getImage(const string& imageIdentifier) {
  auto it = imageMap.find(imageIdentifier);
  if (it == imageMap.end()) {
    logW(LL_CRIT, "attempt to load unloaded image w/ identifier: " + imageIdentifier);
    // exit(1);
  }
  return it->second;
}

void controller::updateResolution() {
  setShaderValue("SH_FXAA", "u_resolution",
                 (Vector2){static_cast<float>(getWidth()), static_cast<float>(getHeight())});
  setShaderValue("SH_OVERLAY", "u_resolution",
                 (Vector2){static_cast<float>(getWidth()), static_cast<float>(getHeight())});
  setShaderTexture("SH_OVERLAY", "texture1", framebuffer);

  Image i = GenImageColor(1, getHeight(), WHITE);
  UnloadTexture(nowLineTex);
  nowLineTex = LoadTextureFromImage(i);
  UnloadImage(i);
}

shaderData& controller::getShaderData(const string& shaderIdentifier) {
  auto it = shaderMap.find(shaderIdentifier);
  if (it == shaderMap.end()) {
    logW(LL_CRIT, "attempt to load unloaded shader w/ identifier: " + shaderIdentifier);
    // exit(1);
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
  for (auto& shaderData : shaderMap) {
    shaderData.second.unloadData();
  }
  menu.unloadData();
  image.unloadData();

  voronoi.unloadData();
  fft.generator_join();

  CloseWindow();
}

void controller::beginBlendMode(int a, int b, int c) {
  rlSetBlendFactors(a, b, c);
  BeginBlendMode(BLEND_CUSTOM);
}
void controller::endBlendMode() { EndBlendMode(); }

ACTION
controller::process(ACTION action) {
  // if needed, clear buffer first
  pendingActionValue = -1;
  if (isKeyPressed(KEY_ESCAPE)) {
    buffer.clear();
  }

  // do not overwrite pending event
  if (action != ACTION::NONE) {
    return action;
  }

  // process key buffer
  auto buf_action = buffer.process();
  if (buf_action != ACTION::NONE) {
    if (buf_action == ACTION::NAV_SET_MEASURE || buf_action == ACTION::NAV_PREV_MEASURE ||
        buf_action == ACTION::NAV_NEXT_MEASURE || buf_action == ACTION::CHANGE_MODE) {
      pendingActionValue = buffer.get_pending();
    }
    return buf_action;
  }

  // setup so that the last key in sequence
  // must be pressed last to prevent repeat inputs
  if (isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL)) {
    // open [file, image]
    if (isKeyPressed(KEY_O)) {
      if (isKeyDown(KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) {
        return ACTION::OPEN_IMAGE;
      }
      return ACTION::OPEN;
    }
    // close [file, image]
    if (isKeyPressed(KEY_W)) {
      if (isKeyDown(KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) {
        return ACTION::CLOSE_IMAGE;
      }
      return ACTION::CLOSE;
    }
    // save [save, save as]
    if (isKeyPressed(KEY_S)) {
      if (isKeyDown(KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) {
        return ACTION::SAVE_AS;
      }
      return ACTION::SAVE;
    }
    // reload
    if (isKeyPressed(KEY_R)) {
      return ACTION::RELOAD;
    }
    if (isKeyPressed(KEY_COMMA)) {
      return ACTION::PREFERENCES;
    }
    if (isKeyPressed(KEY_SPACE)) {
      if (!buffer.entry_in_progress()) {
        return ACTION::LIVEPLAY;
      }
    }
    if (isKeyPressed(KEY_I)) {
      return ACTION::INFO;
    }
    if (isKeyPressed(KEY_F)) {
      return ACTION::FILE_INFO;
    }

    if (isKeyPressed(KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT,
                     KEY_NINE)) {
      return ACTION::CHANGE_MODE;
    }
  }

  if ((isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL, KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) &&
      GetMouseWheelMove() != 0) {
    return ACTION::NAV_ZOOM_IMAGE;
  }
  if ((!isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL, KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) &&
      (isKeyPressed(KEY_DOWN, KEY_UP) || GetMouseWheelMove() != 0)) {
    pendingActionValue = isKeyDown(KEY_LEFT_ALT, KEY_RIGHT_ALT);
    if (isKeyPressed(KEY_DOWN) || (GetMouseWheelMove() < 0)) {
      return ACTION::NAV_ZOOM_IN;
    }
    else {
      return ACTION::NAV_ZOOM_OUT;
    }
  }

  if (isKeyDown(KEY_LEFT)) {
    if (isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL)) {
      return ACTION::NAV_PREV_FAST;
    }
    else if (isKeyDown(KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) {
      return ACTION::NAV_PREV_MEASURE;
    }
    else {
      return ACTION::NAV_PREV;
    }
  }

  if (isKeyDown(KEY_RIGHT)) {
    if (isKeyDown(KEY_LEFT_CONTROL, KEY_RIGHT_CONTROL)) {
      return ACTION::NAV_NEXT_FAST;
    }
    else if (isKeyDown(KEY_LEFT_SHIFT, KEY_RIGHT_SHIFT)) {
      return ACTION::NAV_NEXT_MEASURE;
    }
    else {
      return ACTION::NAV_NEXT;
    }
  }

  if (isKeyPressed(KEY_SPACE)) {
    if (!buffer.entry_in_progress()) {
      return ACTION::NAV_SPACE;
    }
  }

  if (isKeyDown(KEY_HOME)) {
    return ACTION::NAV_HOME;
  }
  if (isKeyDown(KEY_END)) {
    return ACTION::NAV_END;
  }
  if (isKeyPressed(KEY_TAB)) {
    return ACTION::PREFERENCES;
  }

  return ACTION::NONE;
}

void controller::beginFrame() {
  BeginDrawing();
  pushTextureMode("FB_BASE", &framebuffer);
}

void controller::endFrame() {
  popTextureMode();
  drawTexturePro(framebuffer.texture, {0, 0, ctr.getWidth(), -ctr.getHeight()},
                 {0, 0, ctr.getWidth(), ctr.getHeight()}, {0.0f, 0.0f}, 0.0f);

  EndDrawing();
}

void controller::updateFrameBuffer() {
  UnloadRenderTexture(framebuffer);
  framebuffer = LoadRenderTexture(ctr.getWidth(), ctr.getHeight());
  SetTextureFilter(framebuffer.texture, TEXTURE_FILTER_POINT);
}

void controller::pushTextureMode(string rtex_id, RenderTexture* rtex_ptr) {
  // stores the current render texture, ends the current mode, and then begins the next render texture mode
  // only ends the current mode if the stack size is NOT zero

  // logQ("call to pushTextureMode() at frame: " + to_string(getFrame()));

  if (!renderStack.empty()) {
    if (rtex_id == renderStack.top().first) {
      (renderStack.top().second.second)++;
      return;
    }
    endTextureMode();
  }

  renderStack.emplace(make_pair(rtex_id, make_pair(rtex_ptr, 1)));

  beginTextureMode(*renderStack.top().second.first);
}

void controller::popTextureMode() {
  // restores the previous render texture, only if stack size is non-zero

  // logQ("call to popTextureMode() at frame: " + to_string(getFrame()));

  if (renderStack.empty()) {
    // TODO: should popping a render texture off an empty stack be a warning??
    // logW(LL_WARN, "attempt to pop render texture with empty render stack");
    return;
  }

  if (renderStack.top().second.second > 1) {
    renderStack.top().second.second--;
    return;
  }

  renderStack.pop();

  // if this was the last item popped off the stack, do nothing

  if (!renderStack.empty()) {
    beginTextureMode(*renderStack.top().second.first);
  }
  else {
    endTextureMode();
  }
}

void controller::update(int offset, double zoom, double& nowLineX) {
  if (!programState) {
    return;
  }

  frameCounter++;
  if (!livePlayState && run && output.isPortOpen()) {
    fileOutput.allow();
  }
  else {
    fileOutput.disallow();
  }
  // always update to prevent notes playing at once
  if (offset < ctr.getLastTime()) {
    fileOutput.updateOffset(offset);
  }

  menu.updateMouse();
  menu.updateRenderStatus();
  updateKeyState();
  updateDimension(nowLineX);
  updateFPS();
  curMeasure = findCurrentMeasure(offset);

  particle.update(zoom);

  if (run) {
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
    psrValue = getRandRange(0, std::numeric_limits<int>::max());
  }

  updateDroppedFiles();
}

void controller::updateFPSCap(bool state) {
  uncapped_fps = state;
  uncapped_fps ? SetTargetFPS(0) : updateFPS(true);
}

void controller::updateFPS(bool bypass) {
  if (!uncapped_fps) {
    if (curMon != GetCurrentMonitor() || bypass) {
      curMon = GetCurrentMonitor();
      SetTargetFPS(GetMonitorRefreshRate(curMon));
    }
  }
}

void controller::updateKeyState() {
  if (WindowShouldClose()) {
    setCloseFlag();
  }
}

void controller::updateDimension(double& nowLineX) {
  if (IsWindowResized()) {
    updateResolution();
    updateFrameBuffer();
    shadow.update();
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
    if (any_of(ext, "mid", "mki")) {
      if (!ctr.open_file.pending()) {
        ctr.open_file.setPending(paths[item]);
      }
    }
    else if (any_of(ext, "png", "jpg")) {
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
  if (setTrackOn.size() < 1 || option.get(OPTION::TRACK_DIVISION_LIVE)) {
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

void controller::criticalSection(bool enter) {
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
  bgColor2 =
      (invert ^ static_cast<bool>(option.get(OPTION::DYNAMIC_LABEL))) ? maximizeDeltaE(bgColor) : bgLight;
}

vector<colorRGB>& controller::findColorSet(int colorMode, bool on) {
  switch (colorMode) {
    case COLOR_PART:
      return (on ? setTrackOn : setTrackOff);
      break;
    case COLOR_VELOCITY:
      return (on ? setVelocityOn : setVelocityOff);
      break;
    case COLOR_TONIC:
      return (on ? setTonicOn : setTonicOff);
      break;
    default:
      return setTrackOn;
  }
}

vector<string> controller::generateMenuLabels(const menuContentType& contentType) {
  switch (contentType) {
    case CONTENT_FILE:
      return text.getStringSet("FILE_MENU_FILE", "FILE_MENU_OPEN_FILE", "FILE_MENU_OPEN_IMAGE",
                               "FILE_MENU_SAVE", "FILE_MENU_SAVE_AS", "FILE_MENU_CLOSE_FILE",
                               "FILE_MENU_CLOSE_IMAGE", "FILE_MENU_RELOAD", "FILE_MENU_EXIT");
    case CONTENT_EDIT:
      return text.getStringSet("EDIT_MENU_EDIT", "EDIT_MENU_ENABLE_SHEET_MUSIC", "EDIT_MENU_PREFERENCES");
    case CONTENT_VIEW:
      return text.getStringSet("VIEW_MENU_VIEW", "VIEW_MENU_DISPLAY_MODE", "VIEW_MENU_SHOW_SONG_TIME",
                               "VIEW_MENU_SHOW_KEY_SIGNATURE", "VIEW_MENU_SHOW_TEMPO",
                               "VIEW_MENU_HIDE_NOW_LINE", "VIEW_MENU_HIDE_MEASURE_LINE",
                               "VIEW_MENU_HIDE_MEASURE_NUMBER", "VIEW_MENU_HIDE_BACKGROUND",
                               "VIEW_MENU_HIDE_FPS");
    case CONTENT_DISPLAY:
      return text.getStringSet("DISPLAY_MENU_DEFAULT", "DISPLAY_MENU_LINE", "DISPLAY_MENU_PULSE",
                               "DISPLAY_MENU_BALL", "DISPLAY_MENU_FFT", "DISPLAY_MENU_VORONOI",
                               "DISPLAY_MENU_LOOP");
    case CONTENT_SONG:
      return text.getStringSet("SONG_MENU_RELATIVE", "SONG_MENU_ABSOLUTE");
    case CONTENT_MIDI:
      return text.getStringSet("MIDI_MENU_MIDI", "MIDI_MENU_INPUT", "MIDI_MENU_OUTPUT",
                               "MIDI_MENU_ENABLE_LIVE_PLAY");
    case CONTENT_INPUT:
      return {""};
    case CONTENT_OUTPUT:
      return {""};
    case CONTENT_COLOR:
      return text.getStringSet("COLOR_MENU_COLOR", "COLOR_MENU_COLOR_BY", "COLOR_MENU_COLOR_SCHEME",
                               "COLOR_MENU_SWAP_COLORS", "COLOR_MENU_CYCLE_COLORS",
                               "COLOR_MENU_INVERT_COLOR_SCHEME");
    case CONTENT_SCHEME:
      return text.getStringSet("SCHEME_MENU_PART", "SCHEME_MENU_VELOCITY", "SCHEME_MENU_TONIC");
    case CONTENT_INFO:
      return text.getStringSet("INFO_MENU_INFO", "INFO_MENU_PROGRAM_INFO", "INFO_MENU_FILE_INFO",
                               "INFO_MENU_HELP");
    case CONTENT_PALETTE:
      return text.getStringSet("PALETTE_MENU_DEFAULT", "PALETTE_MENU_FROM_BACKGROUND");
    case CONTENT_RIGHT:
      return text.getStringSet("RIGHT_MENU_INFO", "RIGHT_MENU_CHANGE_PART_COLOR", "RIGHT_MENU_SET_TONIC");
    case CONTENT_COLORSELECT:
      return text.getStringSet("COLORSELECT_COLOR_SELECT");
    default:
      logW(LL_WARN, "invalid menu label type -", contentType);
      return {};
  }
}

fileType controller::getFileType() const {
  if (getLiveState()) {
    return FILE_NONE;
  }
  return fType;
}
string controller::getFilePath() const {
  if (getLiveState()) {
    return "";
  }
  return fPath.substr(fPath.find_last_of("/\\") + 1);
}
string controller::getFileFullPath() const {
  if (getLiveState()) {
    return "";
  }
  return fPath;
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
    return option.get(OPTION::TRACK_DIVISION_LIVE) ? 2 : 1;
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

int controller::getTempo(int idx) const {
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

int controller::getCurrentMeasure() const { return curMeasure; }

int controller::findCurrentMeasure(int pos) const {
  if (livePlayState) {
    // TODO: reimplement when measures are added to live input data
    return 0;
  }
  else {
    return file.findMeasure(pos);
  }
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

string controller::getTempoLabel(int offset) const {
  if (livePlayState) {
    return "";
  }

  if (file.measureMap.size() == 0) {
    return "";
  }

  if (offset == 0) {
    offset = 1;
  }

  return to_string(getTempo(offset)) + " BPM";
}

string controller::getKeySigLabel(int offset) const {
  if (livePlayState) {
    // TODO: detect key signature from already-played notes
    // return "NULL";
    return "";
  }

  if (file.measureMap.size() == 0) {
    return "";
  }

  if (offset == 0) {
    offset = 1;
  }

  // int KSOffset =
  // file.measureMap[findCurrentMeasure(offset)-1].currentKey.getIndex();
  // logQ("the current measure", findCurrentMeasure(offset), "has key signature
  // offset", KSOffset);
  return file.measureMap[findCurrentMeasure(offset) - 1].currentKey.getLabel();
}

void controller::clear() {
  midiData.clear();
  file.clear();
  runTime = 0;
  pauseTime = 0;
  bgColor = colorRGB(0, 0, 0);
  fType = FILE_NONE;
  fPath = "";
  fileOutput.disallow(true);
}

void controller::load(string path, bool& nowLine, bool& showFPS, bool& showImage, bool& showKey,
                      bool& showTempo, bool& sheetMusicDisplay, bool& measureLine, bool& measureNumber,
                      int& colorMode, int& displayMode, int& songTimeType, int& tonicOffset,
                      double& zoomLevel) {
  if (!isValidPath(path, PATH_DATA)) {
    logW(LL_WARN, "invalid path:", path);
    return;
  }

  auto start = std::chrono::high_resolution_clock::now();

  if (isValidPath(path, PATH_MKI)) {
    logW(LL_INFO, "load MKI:", path);
    // open input file
    ifstream input(path, std::ifstream::in | std::ios::binary);
    input.imbue(std::locale::classic());

    if (!input) {
      logW(LL_WARN, "unable to load MKI: ", path);
      return;
    }

    std::filesystem::path p{path};
    int file_size = std::filesystem::file_size(p);

    char byteBuf = 0;
    int byteCtr = 0;

    auto readByte = [&]() {
      byteCtr++;
      if (!input.eof()) {
        input.read(&byteBuf, 1);
        return true;
      }
      logW(LL_WARN, "invalid MKI file @ byte", byteCtr);
      return false;
    };

    auto readUntil = [&](int limit) {
      while (byteCtr <= limit) {
        readByte();
      }
    };
    // auto debugByte = [&]() {
    // #ifndef NODEBUG
    // std::bitset<8> binRep(byteBuf);
    // logQ(binRep);
    // #endif
    //};

    // #define readByte(); if(!readByte()) { return; }

    // 0x00
    readByte();
    int mki_major = static_cast<int>((byteBuf >> 4) & 0b00001111);
    char mki_minor = 96 + static_cast<char>(byteBuf & 0b00001111);
    string mki_ver_string = to_string(mki_major) + mki_minor;
    logW(LL_INFO, "MKI v" + mki_ver_string);

    // 0x01
    readByte();

    nowLine = byteBuf & (1 << 7);
    showFPS = byteBuf & (1 << 6);
    showImage = byteBuf & (1 << 5);
    sheetMusicDisplay = byteBuf & (1 << 4);
    measureLine = byteBuf & (1 << 3);
    measureNumber = byteBuf & (1 << 2);

    bool imageExists = byteBuf & (1 << 1);

    showKey = byteBuf & (1 << 0);

    // 0x02:[7:7] - now line
    // 0x02:[6:0] - reserved
    readByte();
    showTempo = byteBuf & (1 << 7);

    // 0x03-0x09
    readUntil(0x09);

    // 0x0A
    readByte();

    colorMode = (byteBuf >> 4) & 0xF;
    displayMode = byteBuf & 0xF;

    // 0x0B
    readByte();
    // debugByte();

    songTimeType = (byteBuf >> 4) & 0xF;
    tonicOffset = byteBuf & 0x0F;

    // 0x0C-0x0F - reserved
    readUntil(0x0F);

    // 0x10-0x2F - user preferences

    // 0x10:[7:7] - track division (live)
    // 0x10:[6:6] - track division (midi)
    // 0x10:[5:5] - set hand range
    // 0x10:[4:4] - set darken image
    // 0x10:[3:3] - set CIE function
    // 0x10:[2:2] - dynamic label
    // 0x10:[1:1] - particle
    // 0x10:[0:0] - scale velocity
    readByte();
    ctr.option.set(OPTION::TRACK_DIVISION_LIVE, (byteBuf >> 7) & 0x1);
    ctr.option.set(OPTION::TRACK_DIVISION_MIDI, (byteBuf >> 6) & 0x1);
    ctr.option.set(OPTION::SET_HAND_RANGE, (byteBuf >> 5) & 0x1);
    ctr.option.set(OPTION::SET_DARKEN_IMAGE, (byteBuf >> 4) & 0x1);
    ctr.option.set(OPTION::SET_CIE_FUNCTION, (byteBuf >> 3) & 0x1);
    ctr.option.set(OPTION::DYNAMIC_LABEL, (byteBuf >> 2) & 0x1);
    ctr.option.set(OPTION::PARTICLE, (byteBuf >> 1) & 0x1);
    ctr.option.set(OPTION::SCALE_VELOCITY, (byteBuf >> 0) & 0x1);

    // 0x11:[7:7] - shadow
    // 0x11:[6:6] - now line overlay blend mode
    // 0x11:[5:5] - use level control
    // 0x11:[4:4] - limit fps
    // 0x11:[3:0] - reserved
    readByte();
    ctr.option.set(OPTION::SHADOW, (byteBuf >> 7) & 0x1);
    ctr.option.set(OPTION::NOW_LINE_USE_OVERLAY, (byteBuf >> 6) & 0x1);
    ctr.option.set(OPTION::USE_LEVEL_CONTROL, (byteBuf >> 5) & 0x1);
    ctr.option.set(OPTION::LIMIT_FPS, (byteBuf >> 4) & 0x1);

    // 0x13:[7:2] - hand range (6 bits)
    // 0x13:[1:0] - CIE function (2 bits)
    readByte();
    ctr.option.set(OPTION::HAND_RANGE, (byteBuf >> 2) & 0b00111111);
    ctr.option.set(OPTION::CIE_FUNCTION, (byteBuf >> 0) & 0b00000011);

    // 0x14:[7:0] - bw channel
    readByte();
    ctr.option.set(OPTION::DARKEN_IMAGE, static_cast<uint8_t>(byteBuf));
    // 0x15:[7:0] - r channel
    readByte();
    ctr.option.set(OPTION::LEVEL_CONTROL_R, static_cast<uint8_t>(byteBuf));
    // 0x16:[7:0] - g channel
    readByte();
    ctr.option.set(OPTION::LEVEL_CONTROL_G, static_cast<uint8_t>(byteBuf));
    // 0x17:[7:0] - b channel
    readByte();
    ctr.option.set(OPTION::LEVEL_CONTROL_B, static_cast<uint8_t>(byteBuf));

    // 0x18-0x19 - shadow angle
    char shadowAngleBuf[2] = {0};
    for (int i = 0; i < 2; ++i) {
      readByte();
      // debugByte();
      shadowAngleBuf[i] = byteBuf;
    }
    ctr.option.set(OPTION::SHADOW_ANGLE, *(reinterpret_cast<uint16_t*>(shadowAngleBuf)));

    // 0x1A:[7:2] - shadow distance (6 bits)
    // 0x1A:[1:0] - reserved
    readByte();
    ctr.option.set(OPTION::SHADOW_DISTANCE, (byteBuf >> 2) & 0b00111111);

    // 0x1B-0x2F - reserved
    readUntil(0x2F);

    // 0x30-0x3F - reserved
    readUntil(0x3F);

    // 0x40-0x44 - zoom level
    char zoomBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      // debugByte();
      zoomBuf[i] = byteBuf;
    }

    zoomLevel = *(reinterpret_cast<float*>(zoomBuf));
    // logQ("READ ZOOMLEVEL", zoomLevel);

    // logQ(zoomLevel);

    // position need not exceed 16 bits
    // 0x45-0x46 - image position (x)
    // 0x47-0x48 - image position (y)
    // 0x49-0x4C - scale
    // 0x4D-0x50 - default scale
    // 0x51-0x54 - mean value
    // 0x55-0x58 - color count

    // if no image exists, all values are left zero

    int16_t i_posx = 0;
    int16_t i_posy = 0;
    float i_scale = 0;
    float i_def_scale = 0;
    float i_mean_v = 0;
    int i_num_col = 0;

    if (imageExists) {
      // 0x45-0x46 - image position (x)
      char imageBuf[2] = {0};
      for (int i = 0; i < 2; ++i) {
        readByte();
        // debugByte();
        imageBuf[i] = byteBuf;
      }
      i_posx = *(reinterpret_cast<uint16_t*>(imageBuf));

      // 0x47-0x48 - image position (y)
      // char imageBuf[4] = {0};
      for (int i = 0; i < 2; ++i) {
        readByte();
        // debugByte();
        imageBuf[i] = byteBuf;
      }
      i_posy = *(reinterpret_cast<uint16_t*>(imageBuf));

      // 0x49-0x4C - scale
      char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        // debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_scale = *(reinterpret_cast<float*>(imageBuf2));

      // 0x4D-0x50 - default scale
      // char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        // debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_def_scale = *(reinterpret_cast<float*>(imageBuf2));

      // 0x51-0x54 - mean value
      // char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        // debugByte();
        imageBuf2[i] = byteBuf;
      }
      i_mean_v = *(reinterpret_cast<float*>(imageBuf2));

      // 0x55-0x58 - color count
      // char imageBuf2[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        // debugByte();
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

    auto readRGB = [&]() {
      readByte();
      uint8_t r = *reinterpret_cast<uint8_t*>(&byteBuf);
      readByte();
      uint8_t g = *reinterpret_cast<uint8_t*>(&byteBuf);
      readByte();
      uint8_t b = *reinterpret_cast<uint8_t*>(&byteBuf);

      colorRGB col(r, g, b);

      return col;
    };

    // 0x59-0x7C - tonic (on) colors
    // 0x7D-0xA0 - tonic (off) colors
    for (auto& col : setTonicOn) {
      col = readRGB();
    }
    for (auto& col : setTonicOff) {
      col = readRGB();
    }

    // 0xA1-0x221  velocity (on) colors
    // 0x222-0x3A1 velocity (off) colors
    for (auto& col : setVelocityOn) {
      col = readRGB();
    }
    for (auto& col : setVelocityOff) {
      col = readRGB();
    }

    // 0x3A2-0x3A5 - background color
    bgColor = readRGB();
    // logQ("READ COLOR", bgColor);

    // 0x3A6-0x3A9 - track size marker (n)
    char trackSizeBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      // debugByte();
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

    // 0x3AA-0x3AA+(n*3)       track (on) colors
    // 0x3AA+(n*3)-0x3AA+(n*6) track (off) colors
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
      // debugByte();
      midiSizeBuf[i] = byteBuf;
    }

    int midiSize = *reinterpret_cast<int*>(midiSizeBuf);
    // logQ("MIDI size is:", midiSize);
    if (midiSize > file_size - byteCtr) {
      logW(LL_WARN, "corrupted MKI file - embedded midi size (" + to_string(midiSize) + ")",
           "is larger than remaining file size (" + to_string(file_size - byteCtr) + ")");
      clear();
      return;
    }

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
        // debugByte();
        imageFormatBuf[i] = byteBuf;
      }

      // error handling of image format handled in loader function
      int imageFormat = *reinterpret_cast<int*>(&imageFormatBuf);
      // logQ("READ IMGFORMAT", imageFormat);

      // next 4 bytes is image size
      char imageSizeBuf[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        // debugByte();
        imageSizeBuf[i] = byteBuf;
      }

      int imageSize = *reinterpret_cast<int*>(&imageSizeBuf);
      if (imageSize > file_size - byteCtr) {
        logW(LL_WARN, "corrupted MKI file - embedded image size (" + to_string(imageSize) + ")",
             "is larger than remaining file size (" + to_string(file_size - byteCtr) + ")");
        clear();
        return;
      }

      stringstream imageData;

      for (auto i = 0; i < imageSize; ++i) {
        readByte();
        imageData.write(&byteBuf, sizeof(byteBuf));
      }

      // logQ(imageData.str());
      //  finally load the image
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
    logW(LL_INFO, "load MIDI:", path);

    file.load(path, midiData);

    getColorScheme(KEY_COUNT, setVelocityOn, setVelocityOff);
    getColorScheme(TONIC_COUNT, setTonicOn, setTonicOff);
    getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);

    // last, set non-MKI loaded flag
    fType = FILE_MIDI;
  }

  fPath = path;
  fileOutput.load(file.message);
  particle.end_emission();

  debug_time(start, "load");
}

void controller::save(string path, bool nowLine, bool showFPS, bool showImage, bool showKey, bool showTempo,
                      bool sheetMusicDisplay, bool measureLine, bool measureNumber, int colorMode,
                      int displayMode, int songTimeType, int tonicOffset, double zoomLevel) {
  // open output file
  ofstream output(path, std::ofstream::out | std::ofstream::trunc | std::ios::binary);
  output.imbue(std::locale::classic());

  if (!output) {
    logW(LL_WARN, "unable to save file to", path);
    return;
  }

  int byteCtr = 0;

  const uint8_t emptyByte = 0;

  auto writeByte = [&](auto data) {
    byteCtr += sizeof(data);

    output.write(reinterpret_cast<const char*>(&data), sizeof(data));
  };

  auto writeUntil = [&](int limit) {
    while (byteCtr <= limit) {
      writeByte(emptyByte);
    }
  };

  // 0x00:[7:4] - MKI_VER_MAJOR
  // 0x00:[3:0] - MKI_VER_MINOR

  uint8_t byte0 = 0;
  byte0 |= ((static_cast<uint8_t>(MKI_VER_MAJOR) & 000001111) << 4);
  byte0 |= (static_cast<uint8_t>(MKI_VER_MINOR) & 0b00001111);

  writeByte(byte0);

  // 0x01:[7:7] - now line
  // 0x01:[6:6] - fps display
  // 0x01:[5:5] - image display
  // 0x01:[4:4] - sheet music display
  // 0x01:[3:3] - measure line display
  // 0x01:[2:2] - measure number display
  // 0x01:[1:1] - image existence
  // 0x01:[0:0] - show key signature
  uint8_t byte1 = 0;
  byte1 |= (nowLine << 7);
  byte1 |= (showFPS << 6);
  byte1 |= (showImage << 5);
  byte1 |= (sheetMusicDisplay << 4);
  byte1 |= (measureLine << 3);
  byte1 |= (measureNumber << 2);
  byte1 |= (image.exists() << 1);
  byte1 |= (showKey << 0);

  // logQ(byte0);
  writeByte(byte1);

  // 0x02:[7:7] - now line
  // 0x02:[6:0] - reserved
  uint8_t byte2 = 0;
  byte2 |= (showTempo << 7);
  writeByte(byte2);

  // 0x02-0x09 - reserved
  writeUntil(0x09);

  // 0x0A:[7:4] - scheme color type
  // 0x0A:[3:0] - display type
  uint8_t byte10 = 0;

  byte10 |= (static_cast<uint8_t>(colorMode) << 4);
  byte10 |= (static_cast<uint8_t>(displayMode) & 0xF);

  writeByte(byte10);

  // 0x0B:[7:4] - song time display type
  // 0x0B:[3:0] - tonic offset
  uint8_t byte11 = 0;

  byte11 |= (static_cast<uint8_t>(songTimeType) << 4);
  byte11 |= (static_cast<uint8_t>(tonicOffset) & 0xF);

  writeByte(byte11);

  // 0x0C-0x0F - reserved
  writeUntil(0x0F);

  // 0x10-0x2F - user preferences
  //
  // 0x10:[7:7] - track division (live)
  // 0x10:[6:6] - track division (midi)
  // 0x10:[5:5] - set hand range
  // 0x10:[4:4] - set darken image
  // 0x10:[3:3] - set CIE function
  // 0x10:[2:2] - dynamic label
  // 0x10:[1:1] - particle
  // 0x10:[0:0] - scale velocity
  uint8_t pref1 = 0;
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::TRACK_DIVISION_LIVE)) << 7);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::TRACK_DIVISION_MIDI)) << 6);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SET_HAND_RANGE)) << 5);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SET_DARKEN_IMAGE)) << 4);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SET_CIE_FUNCTION)) << 3);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::DYNAMIC_LABEL)) << 2);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::PARTICLE)) << 1);
  pref1 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SCALE_VELOCITY)) << 0);

  writeByte(pref1);

  // 0x11:[7:7] - shadow
  // 0x11:[6:6] - now line overlay blend mode
  // 0x11:[5:5] - use level control
  // 0x11:[4:4] - limit fps
  // 0x11:[3:0] - reserved
  uint8_t pref2 = 0;
  pref2 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SHADOW)) << 7);
  pref2 |= (static_cast<uint8_t>(ctr.option.get(OPTION::NOW_LINE_USE_OVERLAY)) << 6);
  pref2 |= (static_cast<uint8_t>(ctr.option.get(OPTION::USE_LEVEL_CONTROL)) << 5);
  pref2 |= (static_cast<uint8_t>(ctr.option.get(OPTION::LIMIT_FPS)) << 4);

  writeByte(pref2);

  // 0x13:[7:2] - hand range (6 bits)
  // 0x13:[1:0] - CIE function (2 bits)
  uint8_t pref3 = 0;
  pref3 |= (static_cast<uint8_t>(ctr.option.get(OPTION::HAND_RANGE)) << 2);
  pref3 |= (static_cast<uint8_t>(ctr.option.get(OPTION::CIE_FUNCTION)) << 0);

  writeByte(pref3);

  // 0x14:[7:0] - bw channel
  // 0x15:[7:0] - r channel
  // 0x16:[7:0] - g channel
  // 0x17:[7:0] - b channel
  writeByte(static_cast<uint8_t>(ctr.option.get(OPTION::DARKEN_IMAGE)));
  writeByte(static_cast<uint8_t>(ctr.option.get(OPTION::LEVEL_CONTROL_R)));
  writeByte(static_cast<uint8_t>(ctr.option.get(OPTION::LEVEL_CONTROL_G)));
  writeByte(static_cast<uint8_t>(ctr.option.get(OPTION::LEVEL_CONTROL_B)));

  // 0x18-0x19 - shadow angle
  writeByte(static_cast<uint16_t>(ctr.option.get(OPTION::SHADOW_ANGLE)));

  // 0x1A:[7:2] - shadow distance (6 bits)
  // 0x1A:[1:0] - reserved
  uint8_t pref4 = 0;
  pref4 |= (static_cast<uint8_t>(ctr.option.get(OPTION::SHADOW_DISTANCE)) << 2);

  writeByte(pref4);

  // 0x1B-0x2F - reserved
  writeUntil(0x2F);

  // 0x30-0x3F - reserved
  writeUntil(0x3F);

  // 0x40-0x44 - zoom level (4bit float (cast from double))
  float zlf = static_cast<float>(zoomLevel);
  // logQ(static_cast<float>(zoomLevel));
  writeByte(zlf);

  // position need not exceed 16 bits
  // 0x45-0x46 - image position (x)
  // 0x47-0x48 - image position (y)
  // 0x49-0x4C - scale
  // 0x4D-0x50 - default scale
  // 0x51-0x54 - mean value
  // 0x55-0x58 - color count

  // if no image exists, all values are left zero

  if (image.exists()) {
    int16_t x = static_cast<int16_t>(image.position.x);
    int16_t y = static_cast<int16_t>(image.position.y);
    writeByte(x);
    writeByte(y);

    float i_scale = image.scale;
    float i_defaultScale = image.defaultScale;
    float i_meanV = image.meanV;

    writeByte(i_scale);
    writeByte(i_defaultScale);
    writeByte(i_meanV);
    writeByte(image.numColors);
  }
  else {
    // needed to maintain file block formatting
    for (int i = 0; i < imageBlockSize; ++i) {
      writeByte(emptyByte);
    }
  }

  // colorRGB has 3 bytes per object
  // track order (on, off):
  // tonic(12)            -> 72    bytes of tonic color data
  // velocity(128)        -> 768   bytes of velocity color data
  // track(variable)      -> n*3*2 bytes of track color data

  auto writeRGB = [&](colorRGB col) {
    uint8_t r = round(col.r);
    uint8_t g = round(col.g);
    uint8_t b = round(col.b);

    // all items are uint8_t
    writeByte(r);
    writeByte(g);
    writeByte(b);
  };

  // 0x59-0x7C - tonic (on) colors
  // 0x7D-0xA0 - tonic (off) colors
  for (auto col : setTonicOn) {
    writeRGB(col);
  }
  for (auto col : setTonicOff) {
    writeRGB(col);
  }

  // 0xA1-0x221  velocity (on) colors
  // 0x222-0x3A1 velocity (off) colors
  for (auto col : setVelocityOn) {
    writeRGB(col);
  }
  for (auto col : setVelocityOff) {
    writeRGB(col);
  }

  // 0x3A2-0x3A5 - background color
  writeRGB(bgColor);
  // logQ("WRITE COLOR", bgColor);

  // 0x3A6-0x3A9 - track size marker (n)
  // 0x3AA-0x3AA+(n*3)       track (on) colors
  // 0x3AA+(n*3)-0x3AA+(n*6) track (off) colors
  uint32_t trackSetSize = setTrackOn.size();
  // logQ("OUTN", trackSetSize);
  writeByte(trackSetSize);

  for (auto col : setTrackOn) {
    writeRGB(col);
  }
  for (auto col : setTrackOff) {
    writeRGB(col);
  }

  // logQ(midiData.str());

  // get size of midi data
  // midiData.seekg(0, std::ios::end);
  // uint32_t midiSize = midiData.tellg();
  // midiData.seekg(0, std::ios::beg);
  uint32_t midiSize = midiData.str().size();

  // logQ("midisize marker is", sizeof(midiSize), "bytes");
  output.write(reinterpret_cast<const char*>(&midiSize), sizeof(midiSize));
  // logQ("saved MIDI length is", midiSize, "bytes");
  output.write(midiData.str().c_str(), midiData.str().size());

  // in variable length regime, only write if the marker is set at 0x00[1:1]
  // here, no need to check marker, just check for existence of a loaded image
  if (image.exists()) {
    // first write image type (4bytes)
    output.write(reinterpret_cast<const char*>(&image.format), sizeof(image.format));

    // logQ("WRITE IMGFORMAT", image.format);

    // get size of image data
    // image.buf.seekg(0, std::ios::end);
    // uint32_t imageSize = image.buf.tellg();
    // image.buf.seekg(0, std::ios::beg);
    uint32_t imageSize = image.buf.str().size();

    // logQ("saved image length is", imageSize, "bytes");
    output.write(reinterpret_cast<const char*>(&imageSize), sizeof(imageSize));
    // logQ(imageSize);
    // output.write(image.buf.str().c_str(), image.buf.str().size());
    output << image.buf.rdbuf();

    // logQ(image.buf.str());
  }

  fType = FILE_MKI;
  fPath = path;
  logW(LL_INFO, "save MKI:", path);
}

void controller::setCloseFlag() {
  programState = false;
  fileOutput.terminate();
  UnloadTexture(nowLineTex);
}

int controller::getRandRange(int a, int b) {
  if (a > b) {
    std::swap(a, b);
  }
  std::uniform_int_distribution<int> dist(a, b);
  // auto result = std::bind(dist, std::mt19937(gen));
  return dist(gen);
}

double controller::getRandClamp() {
  int sample = getRandRange(0, std::numeric_limits<int>::max());

  return static_cast<double>(sample) / std::numeric_limits<int>::max();
}
