#include <algorithm>
#include "define.h"
#include "image.h"
#include "enum.h"
#include "log.h"

using std::min;
using std::max;

void imageController::load(string path) {
  //logW(LL_INFO, "load image:", path);

  unload();

  img = LoadImage(path.c_str());

  if (img.data == nullptr) {
    logW(LL_WARN, "image with path", path, "failed to load!");
  }


  // file is known to work
  ifstream imageData(path);

  if (path.size() > 4) {
    if (path.substr(path.size() - 4) == ".jpg" || path.substr(path.size()-4) == ".jpeg") {
      imageFormat = IMAGE_JPG;
    }
    else if (path.substr(path.size() - 4) == ".png") {
      imageFormat = IMAGE_PNG;
    }
  }

  buf.str("");
  buf.clear();
  buf << imageData.rdbuf();
  imageData.close();

  process();
}

void imageController::load(stringstream& byteData, int byteSize, int format) {


  unload();

  buf.str("");
  buf.clear();
  buf.write(byteData.str().c_str(), byteData.str().size()); 


  string ext = "";

  switch(format) {
    case IMAGE_PNG:
      ext = ".png";
      break;
    case IMAGE_JPG:
      ext = ".jpg";
      break;
    default:
      logW(LL_WARN, "wrong image format");
      return;
  }

  img = LoadImageFromMemory(ext.c_str(), reinterpret_cast<const unsigned char*>(byteData.str().c_str()), byteSize);

  process();
}

void imageController::process() {

  // find default scale
  if (img.width > ctr.getWidth() || img.height > ctr.getHeight()) {
    scale = 1.0/max((float)img.width/ctr.getWidth(), (float)img.height/ctr.getHeight());
    defaultScale = scale;
  } 

  meanV = 0;
  numColors = 0;

  position = {0,0};


  imgTex = LoadTextureFromImage(img);
  SetTextureFilter(imgTex, TEXTURE_FILTER_BILINEAR);
  
  isLoaded = true;

  createRawData();
}
void imageController::unload() {
 if (isLoaded) {
    UnloadImage(img);
    UnloadTexture(imgTex);
    
    position = {0, 0};
    offset = {0, 0};
    base = {0, 0};
    scale = 1;
    defaultScale = 1;

    meanV = 0;
    numColors = 0;

    isLoaded = false;
    rawPixelData.clear();

    imageFormat = IMAGE_NONE;
    buf.str("");
    buf.clear();
  }
}

void imageController::draw() {
  if (isLoaded) {
    DrawTextureEx(imgTex, {(float)position.x + (float)offset.x, (float)position.y + (float)offset.y}, 0, scale, WHITE);
    //DrawRectangle((float)position.x + (float)offset.x, (float)position.y + (float)offset.y, 
                  //getWidth(), getHeight(), {100,100,100,100});
  }
}

void imageController::updatePosition() {
  if (canMove) {
    float xBounds = min(max(0, (int)ctr.getMousePosition().x), ctr.getWidth());
    float yBounds = min(max(ctr.topHeight, (int)ctr.getMousePosition().y), ctr.getHeight()); 

    offset.x = xBounds - base.x;
    offset.y = yBounds - base.y;

    //offset.x = min(max(0, (int)ctr.getMousePosition().x - base.x), ctr.getWidth());
    //offset.y = min(max(ctr.barHeight, (int)ctr.getMousePosition().y - base.y), ctr.getHeight());
  }
}


void imageController::updateBasePosition() {
  canMove = true;
  
  base.x = ctr.getMousePosition().x;
  base.y = ctr.getMousePosition().y;
}

void imageController::finalizePosition() {
  if (canMove) {
    canMove = false;
    position.x += offset.x;
    position.y += offset.y;
    offset = {0, 0};
  }

}

void imageController::changeScale(float scaleOffset) { 
  scale = min(max(0.1f*defaultScale, (float)scale + scaleOffset* scale/defaultScale), 10.0f*defaultScale);
  //logQ("newscale", scale);
}

vector<kMeansPoint> imageController::getRawData() {
  return rawPixelData;
}

void imageController::createRawData() {
  rawPixelData.clear();

  Image copy = ImageCopy(img);
  constexpr int baseWidth = 100;
  ImageResizeNN(&copy, baseWidth, baseWidth * (float)img.width/img.height);
  
  vector<colorRGB> uniqueColors;

  if (isLoaded) {
    // use original image values to prevent effects from scaling
    for (int x = 0; x < copy.width; ++x) {
      for (int y = 0; y < copy.height; ++y) {
        //logQ(colorRGB(GetImageColor(copy, x, y))); 
        Color tmpColorR = GetImageColor(copy, x, y);
        colorRGB tmpColor = {(double)tmpColorR.r, (double)tmpColorR.g, (double)tmpColorR.b};
        meanV += tmpColor.getHSV().v;
        rawPixelData.push_back(kMeansPoint(tmpColor));
        
        if (uniqueColors.size() < MAX_UNIQUE_COLORS && 
            find(uniqueColors.begin(), uniqueColors.end(), tmpColor) == uniqueColors.end()) {
          uniqueColors.push_back(tmpColor);
        }

      }
    } 
  }
  
  meanV /= (copy.width*copy.height);
  numColors = uniqueColors.size();
  //logQ("numC", numColors);

  UnloadImage(copy);

}

float imageController::getMeanValue() {
  return meanV;
}

int imageController::getNumColors() {
  return numColors;
}