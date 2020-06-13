#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#include "note.h"

using std::string;

class window {
  public:
    const static int WIDTH = WIN_WIDTH;
    const static int HEIGHT = WIN_HEIGHT;
     
    window(string title);
    bool init();
    unsigned char eventHandler(SDL_Event &event);
    bool pointVisible(int x, int y);
    bool noteVisible(note n);
    int getWidth();
    int getHeight();
     
    void setPixelRGB(int x, int y, Uint8 r, Uint8 g, Uint8 b);
    Uint8* getPixelRGB(int x, int y);
    
    void update();
    void clearBuffer();
    void terminate();
  private:
    string title;
    SDL_Window* windowA;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    Uint32* buffer;
};

class colorRGB {
  public:
    colorRGB();
    colorRGB(char red, char green, char blue);

    void setRGB(char red, char green, char blue);

    char r;
    char g;
    char b;
};


#endif
