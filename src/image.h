 #pragma once

#include <raylib.h>
#include <string>
#include "box.h"

using std::string;

class imageController {
  public:
    imageController() : img(),  imgTex(), isLoaded(false), position({0,0}), rotation(0), scale(1) {};

    void load(string path);
    void unload();

    void draw();

  private:
    
    Image img;
    Texture2D imgTex;
    
    bool isLoaded;

    point position;
    float rotation;
    float scale;
    

};
