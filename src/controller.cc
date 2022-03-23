#include "controller.h"

using std::ofstream;
using std::stringstream;

void controller::updateKeyState() {
  if (WindowShouldClose()) {
    programState = false;
  }
}

void controller::toggleLivePlay() {
  if (setTrackOn.size() < 1) {
    getColorScheme(2, setTrackOn, setTrackOff);
  }
  livePlayState = !livePlayState;
  if (livePlayState) {
    livePlayOffset = 0;
    notes = &liveInput.noteStream.notes;
  }
  else {
    // when turning off live input, revert to previously loaded file info
    // also, disable and clear the live input event queue
    liveInput.resetInput();
    notes = &file.notes;
  }
  // ensure sufficient track colors
  getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);
}

int controller::getTrackCount() {
  if (livePlayState) {
    return 1;
  }
  return file.getTrackCount();
}

int controller::getNoteCount() {
  if (livePlayState) {
    return liveInput.getNoteCount();
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

void controller::load(string path) {

  // TODO: implement type and filter based on type

  
  //logW(LL_INFO, "load midi:", filename);
  file.load(path, midiData);
  getColorScheme(KEY_COUNT, setVelocityOn, setVelocityOff);
  getColorScheme(TONIC_COUNT, setTonicOn, setTonicOff);
  getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);
}

void controller::save(string path, 
                      bool nowLine, bool showFPS, bool showImage, bool sheetMusicDisplay,
                      bool measureLine, bool measureNumber, 

                      int colorMode, int displayMode,

                      int songTimeType, int tonicOffset, 

                      double zoomLevel) {

  // open output file
  ofstream output(path, std::ofstream::out | std::ofstream::trunc);

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
  // 0x00:[1:0] - reserved

  uint8_t byte0 = 0;
  byte0 |= (nowLine << 7);
  byte0 |= (showFPS << 6);
  byte0 |= (showImage << 5);
  byte0 |= (sheetMusicDisplay << 4);
  byte0 |= (measureLine << 3);
  byte0 |= (measureNumber << 2);

  logQ(byte0);

  output << byte0;

  
  // 0x01:[7:0] - reserved
  // 0x02:[7:0] - reserved

  output << emptyByte << emptyByte;
  
  // 0x03:[7:4] - scheme color type
  // 0x03:[3:0] - display type
  
  uint8_t byte3 = 0;

  byte3 |= (static_cast<uint8_t>(colorMode) << 4);
  byte3 |= (static_cast<uint8_t>(displayMode) & 0xF);

  output << byte3;
  
  // 0x04:[7:4] - song time display type
  // 0x04:[3:0] - tonic offset
  
  uint8_t byte4 = 0;

  byte4 |= (static_cast<uint8_t>(songTimeType) << 4);
  byte4 |= (static_cast<uint8_t>(tonicOffset) & 0xF);

  output << byte4;
  
  // 0x05:[7:0] - reserved
  // 0x06:[7:0] - reserved
  // 0x07:[7:0] - reserved
  
  output << emptyByte << emptyByte << emptyByte;
  
  // 0x07-0x0A - zoom level (4bit float (cast from double))
  
  // COLOR NOT YET IMPLEMENTED

  // 0x0B-0x0E - image x position (x) 

  // colorRGB has 3 bytes per object
  // track order:
  // velocity(128)        -> 384 bytes of velocity color data
  // tonic(12)            -> 36  bytes of tonic color data 
  // track(variable)      -> n*3 bytes of track color data


  //output << midiData.str();
}

void controller::loadTextures() {
    quarter = LoadTexture("bin/textures/noteQ.png");
    half = LoadTexture("bin/textures/noteH.png");
    whole = LoadTexture("bin/textures/noteW.png");
    flag = LoadTexture("bin/textures/flag.png");

    sharp = LoadTexture("bin/textures/sharp.png");
    flat = LoadTexture("bin/textures/flat.png");
    natural = LoadTexture("bin/textures/natural.png");

    restQ = LoadTexture("bin/textures/restQ.png");
    restE = LoadTexture("bin/textures/restE.png");
    
    treble = LoadTexture("bin/textures/treble.png");
    brace = LoadTexture("bin/textures/brace.png");
    bass = LoadTexture("bin/textures/bass.png");
    
    fontMusic = LoadFontEx("bin/fonts/petaluma.otf", 24, 0, 548);
}

void controller::unloadTextures() {

  UnloadTexture(quarter);
  UnloadTexture(half);
  UnloadTexture(whole);
  UnloadTexture(flag);

  UnloadTexture(sharp);
  UnloadTexture(flat);
  UnloadTexture(natural);
  UnloadTexture(restQ);
  UnloadTexture(restE);
  UnloadTexture(treble);
  UnloadTexture(brace);
  UnloadTexture(bass);
}

void controller::setCloseFlag() {
  programState = false;
}
