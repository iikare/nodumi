 #pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include "box.h"
#include "color.h"

using std::vector;
using std::string;
using std::min;
using std::max;



class imageController {
  public:
    imageController() : img(),  imgTex(), isLoaded(false), canMove(false),
                        position({0,0}), base({0,0}), offset({0,0}), scale(1), defaultScale(1) {};

    void load(string path);
    void unload();

    void draw();
    void changeScale(float scaleOffset);

    vector<colorRGB> getRawData();


    bool exists() { return isLoaded; }


    int getX() { return position.x + offset.x; }
    int getY() { return position.y + offset.y; }
    int getWidth() { return isLoaded ? img.width * scale : -1; }
    int getHeight() { return isLoaded ? img.height * scale : -1; }

    
    void updatePosition();
    void updateBasePosition();
    void finalizePosition();

  private:
    
    Image img;
    Texture2D imgTex;
    
    bool isLoaded;
    bool canMove;

    point position;
    point base;
    point offset;
    float scale;
    float defaultScale;
    

};
