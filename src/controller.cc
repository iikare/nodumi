#include "controller.h"

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
  file.load(path);
  getColorScheme(KEY_COUNT, setVelocityOn, setVelocityOff);
  getColorScheme(TONIC_COUNT, setTonicOn, setTonicOff);
  getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);
}

void controller::save(string path) {

  file.midifile.write(path + ".mid");

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
