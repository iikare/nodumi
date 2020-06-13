#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>

#include "note.h"

using std::string;

class window {
  public:
    const static int WIDTH = 1280;
    const static int HEIGHT = 720;
     
    window(string title);
    bool init();
    unsigned char eventHandler(SDL_Event &event, Sint32 &shiftX);
    bool pointVisible(int x, int y);
    bool noteVisible(note n);
    int getWidth();
    int getHeight();
     
    void setPixelRGB(int x, int y, Uint8 r, Uint8 g, Uint8 b);
    void setPixelHex(int x, int y, unsigned char hex);
    Uint8* getPixelRGB(int x, int y);
    unsigned char getPixelHex(int x, int y);
    
    void blur(int intensity);
    void swapBuffer();
    void update();
    void clearBuffer();
    void terminate();
  private:
    string title;
    SDL_Window* windowA;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    
    Uint32* buffer;
    Uint32* backBuffer;
    unsigned char* colbuf;
};

class colorRGB {
  public:
    colorRGB() : r(0), g(0), b(0) {}
    colorRGB(char red, char green, char blue) : r(red), g(green), b(blue) {}

    void setRGB(char red, char green, char blue);

    char r;
    char g;
    char b;
};


#endif
