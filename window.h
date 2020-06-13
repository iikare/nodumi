#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include "dpd/SDL_FontCache/SDL_FontCache.h"
#include "note.h"
#include "window.h"

using std::string;

class colorRGB {
  public:
    colorRGB();
    colorRGB(unsigned char red, unsigned char green, unsigned char blue);

    void setRGB(unsigned char red, unsigned char green, unsigned char blue);

    unsigned char r;
    unsigned char g;
    unsigned char b;
};

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

    void renderFont(int x, int y, string text);
     
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
    
    FC_Font* menuFont;
    colorRGB menuColor;
    int fontSize;
};

#endif
