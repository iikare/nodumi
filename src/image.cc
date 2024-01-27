#include "image.h"

#include <algorithm>

#include "define.h"
#include "enum.h"
#include "log.h"
#include "wrap.h"

using std::max;
using std::min;

void imageController::load(const string& path) {
  logW(LL_INFO, "load image:", path);

  unloadData();

  if (!isValidPath(path, PATH_IMAGE)) {
    logW(LL_WARN, "invalid path:", path);
    return;
  }

  image = LoadImage(path.c_str());

  // file is known to work
  ifstream imageData(path);

  string ext = getExtension(path);

  if (ext == "jpg") {
    format = IMAGE_JPG;
  }
  else if (ext == "png") {
    format = IMAGE_PNG;
  }
  else {
    logW(LL_WARN, "unknown format", ext);
    format = IMAGE_NONE;
  }

  buf.str("");
  buf.clear();
  buf << imageData.rdbuf();
  imageData.close();

  process();
}

void imageController::load(stringstream& byteData, int byteSize, int fmt) {
  unloadData();

  buf.str("");
  buf.clear();
  buf.write(byteData.str().c_str(), byteData.str().size());

  string ext = "";

  switch (fmt) {
    case IMAGE_PNG:
      ext = ".png";
      format = IMAGE_PNG;
      break;
    case IMAGE_JPG:
      ext = ".jpg";
      format = IMAGE_JPG;
      break;
    default:
      logW(LL_WARN, "wrong image format:", fmt);
      return;
  }

  image = LoadImageFromMemory(ext.c_str(), reinterpret_cast<const unsigned char*>(byteData.str().c_str()), byteSize);

  process();
}

void imageController::process() {
  // find default scale
  if (image.width > ctr.getWidth() || image.height > ctr.getHeight()) {
    scale = 1.0 / max((double)image.width / ctr.getWidth(), (double)image.height / ctr.getHeight());
    defaultScale = scale;
  }

  meanV = 0;
  numColors = 0;

  position = {0, 0};

  imageTex = LoadTextureFromImage(image);
  SetTextureFilter(imageTex, TEXTURE_FILTER_BILINEAR);

  isLoaded = true;

  createRawData();
}

void imageController::unloadData() {
  if (isLoaded) {
    UnloadImage(image);
    UnloadTexture(imageTex);

    position = {0, 0};
    offset = {0, 0};
    base = {0, 0};
    scale = 1;
    defaultScale = 1;

    meanV = 0;
    numColors = 0;

    isLoaded = false;
    rawPixelData.clear();

    format = IMAGE_NONE;
    buf.str("");
    buf.clear();
  }
}

void imageController::render() {
  if (isLoaded) {
    drawTextureEx(imageTex, Vector2{(float)position.x + (float)offset.x, (float)position.y + (float)offset.y}, 0,
                  scale);

    if (ctr.option.get(OPTION::SET_DARKEN_IMAGE)) {
      int v = ctr.option.get(OPTION::DARKEN_IMAGE);

      constexpr double reduce_ratio = 8.0;
      colorRGB c(v / reduce_ratio, v / reduce_ratio, v / reduce_ratio);

      drawRectangle(static_cast<float>(position.x + offset.x), static_cast<float>(position.y + offset.y),
                    image.width * scale, image.height * scale, c, min(200, v));
    }
  }
}

void imageController::updatePosition() {
  if (canMove) {
    double xBounds = min(max(0, getMousePosition().x), ctr.getWidth());
    double yBounds = min(max(ctr.topHeight, getMousePosition().y), ctr.getHeight());

    offset.x = xBounds - base.x;
    offset.y = yBounds - base.y;

    // offset.x = min(max(0, (int)getMousePosition().x - base.x),
    // ctr.getWidth()); offset.y = min(max(ctr.barHeight,
    // (int)getMousePosition().y - base.y), ctr.getHeight());
  }
}

void imageController::updateBasePosition() {
  canMove = true;

  base.x = getMousePosition().x;
  base.y = getMousePosition().y;
}

void imageController::finalizePosition() {
  if (canMove) {
    canMove = false;
    position.x += offset.x;
    position.y += offset.y;
    offset = {0, 0};
  }
}

void imageController::changeScale(double scaleOffset) {
  double oldScale = scale;
  double oldWidth = getWidth();
  double oldHeight = getHeight();

  scale = min(max(0.1 * defaultScale, scale + scaleOffset * scale / defaultScale), 10.0 * defaultScale);

  double diffScale = scale - oldScale;

  double expX = diffScale * image.width;
  double expY = diffScale * image.height;
  double ratioX = (ctr.getMouseX() - position.x) / oldWidth;
  double ratioY = (ctr.getMouseY() - position.y) / oldHeight;

  // logQ("xy", ratioX, ratioY, expX, expY);

  // if (isKeyDown(KEY_F)) {scale = oldScale; }
  // else {
  position.x -= ratioX * expX;
  position.y -= ratioY * expY;
  //}
}

vector<kMeansPoint> imageController::getRawData() { return rawPixelData; }

void imageController::createRawData() {
  rawPixelData.clear();

  Image copy = ImageCopy(image);
  constexpr int baseWidth = 100;
  ImageResizeNN(&copy, baseWidth, baseWidth * (double)image.width / image.height);

  vector<colorRGB> uniqueColors;

  auto exitCond = [&] {
    meanV /= (copy.width * copy.height);
    numColors = uniqueColors.size();
    // logQ("numC", numColors);

    UnloadImage(copy);
  };

  if (isLoaded) {
    // use original image values to prevent effects from scaling
    for (int x = 0; x < copy.width; ++x) {
      for (int y = 0; y < copy.height; ++y) {
        if (uniqueColors.size() >= MAX_UNIQUE_COLORS) {
          exitCond();
          return;
        }
        // logQ(colorRGB(GetImageColor(copy, x, y)));
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

  exitCond();
}

double imageController::getMeanValue() { return meanV; }

int imageController::getNumColors() { return numColors; }
