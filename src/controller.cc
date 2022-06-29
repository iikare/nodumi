#include "controller.h"
#include <stdlib.h>
#include <bitset>
#include "define.h"


using std::ofstream;
using std::stringstream;
using std::bitset;
using std::make_pair;
using std::move;

// shared across load/save routines
const int imageBlockSize = 20;

void controller::initData(vector<asset>& assets) {
  for (const auto& item : assets) {
    switch(item.assetType) {
      case ASSET_FONT:
        fontMap.insert(make_pair(item.assetName, make_pair(item, map<int, Font>())));
        break;
    }
  }
}

Font* controller::getFont(string id, int size) {
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
    if (tmpFontAsset.assetName == "LELAND") {
      loc = codepointSet.data();
      lim = codepointSet.size();
    }

    //logQ("font lim is", lim);
    //logQ("codepoints are", formatVector(codepointSet));

    Font tmp = LoadFontFromMemory(".otf", tmpFontAsset.data, tmpFontAsset.dataLen, size, loc , lim);

    ////logQ(sceneController->getFontData("LMP_R").assetName);

    SetTextureFilter(tmp.texture, TEXTURE_FILTER_BILINEAR);
    
    fit->second.second.insert(make_pair(size, move(tmp)));
  
    //it = fontMap.find(size);
  }

  return &fit->second.second.find(size)->second;
  
  
}

void controller::unloadData() {
  for (const auto& item : fontMap) {
    for (const auto& font : item.second.second) {
      UnloadFont(font.second);
    }
  }
  ctr.image.unload();
}


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


void controller::load(string path, 
                      bool& nowLine, bool& showFPS, bool& showImage, bool& sheetMusicDisplay,
                      bool& measureLine, bool& measureNumber, 

                      int& colorMode, int& displayMode,

                      int& songTimeType, int& tonicOffset, 

                      double& zoomLevel) {
  // TODO: implement type and filter based on type



  if (isMKI(path)) {
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

    //logQ(zoomLevel);
    
    if (imageExists) {
      
      // TODO: handle image metadata loading here
      for (auto i = 0; i < imageBlockSize; ++i) {
        readByte();
      }

    }
    else {
      // ignore image metadata block otherwise
      for (auto i = 0; i < imageBlockSize; ++i) {
        readByte();
      }
    }

    auto readRGB = [&] () {
      readByte();
      uint8_t r = *reinterpret_cast<uint*>(&byteBuf);
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
    for (auto col : setVelocityOn) {
      col = readRGB();
    }
    for (auto col : setVelocityOff) {
      col = readRGB();
    }
    
    
    // 0x368-0x371 - track size marker (n)
    char trackSizeBuf[4] = {0};
    for (int i = 0; i < 4; ++i) {
      readByte();
      //debugByte();
      trackSizeBuf[i] = byteBuf;
    }

    int trackSetSize = *(reinterpret_cast<int*>(trackSizeBuf));

    setTrackOn.resize(trackSetSize);
    setTrackOff.resize(trackSetSize);

    logQ(trackSetSize);

    // 0x372-0x372+(n*3)       track (on) colors
    // 0x372+(n*3)-0x372+(n*6) track (off) colors
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

    stringstream midiData;

    for (auto i = 0; i < midiSize; ++i) {
      readByte();
      midiData.write(&byteBuf, sizeof(byteBuf));
    }

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
      int imageFormat = *reinterpret_cast<int*>(imageFormatBuf);
      
      // next 4 bytes is image size
      char imageSizeBuf[4] = {0};
      for (int i = 0; i < 4; ++i) {
        readByte();
        //debugByte();
        imageSizeBuf[i] = byteBuf;
      }

      int imageSize = *reinterpret_cast<int*>(imageSizeBuf);
        
      stringstream imageData;

      for (auto i = 0; i < imageSize; ++i) {
        readByte();
        imageData.write(&byteBuf, sizeof(byteBuf));
      }


      // finally load the image
      image.load(imageData, imageSize, imageFormat);

    }

  }
  else {
    logW(LL_INFO, "load midi:", path);

    file.load(path, midiData);

    getColorScheme(KEY_COUNT, setVelocityOn, setVelocityOff);
    getColorScheme(TONIC_COUNT, setTonicOn, setTonicOff);
    getColorScheme(getTrackCount(), setTrackOn, setTrackOff, file.trackHeightMap);
  }
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

  output << byte0;

  
  // 0x01:[7:0] - reserved
  // 0x02:[7:0] - reserved

  output << emptyByte << emptyByte;
  
  // 0x03:[7:4] - scheme color type
  // 0x03:[3:0] - display type
  
  uint8_t byte3 = 0;

  byte3 |= (static_cast<uint8_t>(colorMode) << 4);
  byte3 |= (static_cast<uint8_t>(displayMode) & 0xF);

  //logQ((unsigned int)byte3);

  output << byte3;
  
  // 0x04:[7:4] - song time display type
  // 0x04:[3:0] - tonic offset
  
  uint8_t byte4 = 0;

  byte4 |= (static_cast<uint8_t>(songTimeType) << 4);
  //logQ(bitset<8>(songTimeType));
  //logQ(bitset<8>(byte4));
  byte4 |= (static_cast<uint8_t>(tonicOffset) & 0xF);

  output << byte4;
  
  // 0x05:[7:0] - reserved
  // 0x06:[7:0] - reserved
  // 0x07:[7:0] - reserved
  
  output << emptyByte << emptyByte << emptyByte;
  
  // 0x08-0x0B - zoom level (4bit float (cast from double))
 
  float zlf = static_cast<float>(zoomLevel);


  //logQ("zl", zlf);

  output.write( reinterpret_cast<const char*>(&zlf), sizeof(zlf));
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
    
    output.write(reinterpret_cast<const char*>(&image.scale), sizeof(image.scale));
    output.write(reinterpret_cast<const char*>(&image.defaultScale), sizeof(image.defaultScale));
    output.write(reinterpret_cast<const char*>(&image.meanV), sizeof(image.meanV));
    output.write(reinterpret_cast<const char*>(&image.numColors), sizeof(image.numColors));
  }
  else {
   
    // needed to maintain file block formatting

    for (auto i = 0; i < imageBlockSize; ++i) {
      output << emptyByte;
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
  
  
  // 0x368-0x371 - track size marker (n)
  // 0x372-0x372+(n*3)       track (on) colors
  // 0x372+(n*3)-0x372+(n*6) track (off) colors
  uint32_t trackSetSize = setTrackOn.size();
  output.write(reinterpret_cast<const char*>(&trackSetSize), sizeof(trackSetSize));

  for (auto col : setTrackOn) {
    writeRGB(col);
  }
  for (auto col : setTrackOff) {
    writeRGB(col);
  }

  // get size of midi data
  midiData.seekg(0, std::ios::end);
  uint32_t midiSize = midiData.tellg();
  midiData.seekg(0, std::ios::beg);
 
  logQ("midisize marker is", sizeof(midiSize), "bytes");
  output.write(reinterpret_cast<const char*>(&midiSize), sizeof(midiSize));
  logQ("saved midi stream byte length is:",midiSize);
  output.write(midiData.str().c_str(), midiData.str().size());


  // in variable length regime, only write if the marker is set at 0x00[1:1] 
  // here, no need to check marker, just check image object
  if (image.exists()) {
    // first write image type
    output.write(reinterpret_cast<const char*>(&image.imageFormat), sizeof(image.imageFormat));
    
    // get size of image data
    image.buf.seekg(0, std::ios::end);
    uint32_t imageSize = image.buf.tellg();
    image.buf.seekg(0, std::ios::beg);
    
    output.write(reinterpret_cast<const char*>(&imageSize), sizeof(imageSize));
    //logQ(imageSize);
    output.write(image.buf.str().c_str(), image.buf.str().size());
  }

}

point controller::getMousePosition() { 

  if (IsWindowFocused()) {
    return (point){ GetMouseX(), GetMouseY()}; 
  }
  return {-1,-1};
}

void controller::setCloseFlag() {
  programState = false;
}
