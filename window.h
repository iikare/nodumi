#ifndef SCREEN_H
#define SCREEN_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
    bool mouseVisible();

    int getWidth();
    int getHeight();
    int getMouseX();
    int getMouseY();

    void renderTextToTexture(int x, int y, string text, int fSize);
     
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
    int windowX;
    int windowY;

    SDL_Texture* tTexture;
    SDL_Surface* tSurface;
    SDL_Rect clip;
    int clipX;
    int clipY;

    Uint32* buffer;
    
    TTF_Font* menuFont;
    colorRGB menuColor;
    int fontSize;

    int mouseX;
    int mouseY;
};

#endif
