#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include "box.h"
#include "color.h"
#include "colorgen.h"

using std::vector;
using std::string;
using std::min;
using std::max;
using std::ifstream;
using std::stringstream;



class imageController {
  public:
    imageController() : img(),  imgTex(), isLoaded(false), canMove(false),
                        position({0,0}), base({0,0}), offset({0,0}), scale(1), defaultScale(1), meanV(0), numColors(0),
                        rawPixelData(), imageFormat(IMAGE_NONE), buf() {};

    void load(string path);
    void unload();

    void draw();
    void changeScale(float scaleOffset);

    vector<kMeansPoint> getRawData();
    float getMeanValue();
    int getNumColors();


    bool exists() { return isLoaded; }


    int getX() { return position.x + offset.x; }
    int getY() { return position.y + offset.y; }
    int getWidth() { return isLoaded ? img.width * scale : -1; }
    int getHeight() { return isLoaded ? img.height * scale : -1; }

    
    void updatePosition();
    void updateBasePosition();
    void finalizePosition();

    friend class controller;
  private:
   
    void createRawData();

    Image img;
    Texture2D imgTex;
    
    bool isLoaded;
    bool canMove;

    point position;
    point base;
    point offset;
    float scale;
    float defaultScale;
    float meanV;
    int numColors;

    vector<kMeansPoint> rawPixelData;

    int imageFormat;
    stringstream buf;
    

};
